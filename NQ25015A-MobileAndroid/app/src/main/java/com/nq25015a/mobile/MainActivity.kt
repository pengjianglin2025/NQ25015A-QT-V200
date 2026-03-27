package com.nq25015a.mobile

import android.Manifest
import android.annotation.SuppressLint
import android.app.Application
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.Divider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Slider
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import java.time.LocalDateTime
import java.util.UUID
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow

class MainActivity : ComponentActivity() {
    private val permissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestBlePermissions()
        setContent {
            MaterialTheme {
                val vm: NqAppViewModel = viewModel(
                    factory = NqAppViewModel.factory(application)
                )
                NqApp(vm)
            }
        }
    }

    private fun requestBlePermissions() {
        val permissions = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            arrayOf(
                Manifest.permission.BLUETOOTH_SCAN,
                Manifest.permission.BLUETOOTH_CONNECT
            )
        } else {
            arrayOf(Manifest.permission.ACCESS_FINE_LOCATION)
        }
        permissionLauncher.launch(permissions)
    }
}

data class BleDeviceItem(
    val address: String,
    val name: String
)

data class WorkStatus(
    val state: Int = 0,
    val remainingSec: Int = 0,
    val workSec: Int = 0,
    val pauseSec: Int = 0
)

data class DeviceState(
    val aromaOn: Boolean = false,
    val fanOn: Boolean = false,
    val lightOn: Boolean = false,
    val motorOn: Boolean = false,
    val lockOn: Boolean = false,
    val concentration: Int = 0,
    val mode: Int = 1,
    val timingMinutes: Int = 0,
    val timingConcentration: Int = 1,
    val concentrationCount: Int = 3,
    val workStatus: WorkStatus = WorkStatus(),
    val linkStatus: Int = 0
)

data class UiState(
    val isScanning: Boolean = false,
    val isConnected: Boolean = false,
    val connectedName: String = "",
    val devices: List<BleDeviceItem> = emptyList(),
    val state: DeviceState = DeviceState(),
    val logLines: List<String> = emptyList(),
    val statusText: String = "Idle"
)

class NqAppViewModel(application: Application) : AndroidViewModel(application) {
    private val bleClient = NqBleClient(application.applicationContext) { event ->
        when (event) {
            is BleEvent.Log -> appendLog(event.message)
            is BleEvent.ConnectionChanged -> {
                _uiState.value = _uiState.value.copy(
                    isConnected = event.connected,
                    connectedName = event.name ?: "",
                    statusText = if (event.connected) "Connected" else "Disconnected"
                )
                if (event.connected) requestAll()
            }
            is BleEvent.DevicesFound -> {
                _uiState.value = _uiState.value.copy(devices = event.devices)
            }
            is BleEvent.FrameReceived -> handleFrame(event.data)
        }
    }

    private val _uiState = MutableStateFlow(UiState())
    val uiState = _uiState.asStateFlow()

    fun startScan() {
        _uiState.value = _uiState.value.copy(isScanning = true, statusText = "Scanning")
        bleClient.startScan()
    }

    fun stopScan() {
        bleClient.stopScan()
        _uiState.value = _uiState.value.copy(isScanning = false, statusText = "Scan stopped")
    }

    fun connect(device: BleDeviceItem) = bleClient.connect(device.address)
    fun disconnect() = bleClient.disconnect()
    fun requestAll() = sendFrame(NqProtocol.buildStatusRequest())
    fun syncTime() = sendFrame(NqProtocol.buildTimeSync(LocalDateTime.now()))

    fun setAroma(enabled: Boolean) = sendFrame(NqProtocol.buildDpBooleanWrite(1, enabled))
    fun setConcentration(level: Int) = sendFrame(NqProtocol.buildDpByteWrite(3, level))
    fun setFan(enabled: Boolean) = sendFrame(NqProtocol.buildDpBooleanWrite(4, enabled))
    fun setLock(enabled: Boolean) = sendFrame(NqProtocol.buildDpBooleanWrite(5, enabled))
    fun setLight(enabled: Boolean) = sendFrame(NqProtocol.buildDpBooleanWrite(9, enabled))
    fun setMotor(enabled: Boolean) = sendFrame(NqProtocol.buildDpBooleanWrite(15, enabled))
    fun setMode(mode: Int) = sendFrame(NqProtocol.buildDpByteWrite(25, mode))

    fun setCustomTiming(minutes: Int, concentration: Int) {
        sendFrame(NqProtocol.buildCustomTiming(minutes, concentration))
    }

    private fun sendFrame(frame: ByteArray) {
        appendLog("TX ${frame.toHexString()}")
        bleClient.write(frame)
    }

    private fun handleFrame(frame: ByteArray) {
        appendLog("RX ${frame.toHexString()}")
        val parsed = NqProtocol.parseFrame(frame) ?: return

        when (parsed.command) {
            0x03 -> {
                val link = parsed.payload.firstOrNull()?.toUnsignedInt() ?: 0
                updateState { it.copy(linkStatus = link) }
            }
            0x07 -> parseDpReport(parsed.payload)
            0x1C -> if (parsed.payload.isEmpty()) syncTime()
        }
    }

    private fun parseDpReport(payload: ByteArray) {
        if (payload.size < 4) return
        val dpId = payload[0].toUnsignedInt()
        val len = (payload[2].toUnsignedInt() shl 8) or payload[3].toUnsignedInt()
        if (payload.size < 4 + len) return
        val data = payload.copyOfRange(4, 4 + len)

        when (dpId) {
            1 -> updateState { it.copy(aromaOn = data.firstOrNull() == 1.toByte()) }
            3 -> updateState { it.copy(concentration = data.firstOrNull()?.toUnsignedInt() ?: 0) }
            4 -> updateState { it.copy(fanOn = data.firstOrNull() == 1.toByte()) }
            5 -> updateState { it.copy(lockOn = data.firstOrNull() == 1.toByte()) }
            9 -> updateState { it.copy(lightOn = data.firstOrNull() == 1.toByte()) }
            15 -> updateState { it.copy(motorOn = data.firstOrNull() == 1.toByte()) }
            24 -> if (data.size >= 7) {
                updateState {
                    it.copy(
                        workStatus = WorkStatus(
                            state = data[0].toUnsignedInt(),
                            remainingSec = readU16(data, 1),
                            workSec = readU16(data, 3),
                            pauseSec = readU16(data, 5)
                        )
                    )
                }
            }
            25 -> updateState { it.copy(mode = data.firstOrNull()?.toUnsignedInt() ?: 1) }
            26 -> updateState { it.copy(concentrationCount = data.firstOrNull()?.toUnsignedInt() ?: 3) }
            27 -> if (data.size >= 3) {
                updateState {
                    it.copy(
                        timingMinutes = readU16(data, 0),
                        timingConcentration = data[2].toUnsignedInt()
                    )
                }
            }
        }
    }

    private fun updateState(transform: (DeviceState) -> DeviceState) {
        _uiState.value = _uiState.value.copy(state = transform(_uiState.value.state))
    }

    private fun appendLog(message: String) {
        _uiState.value = _uiState.value.copy(
            logLines = (_uiState.value.logLines + message).takeLast(120)
        )
    }

    companion object {
        fun factory(application: Application): ViewModelProvider.Factory {
            return object : ViewModelProvider.Factory {
                @Suppress("UNCHECKED_CAST")
                override fun <T : androidx.lifecycle.ViewModel> create(modelClass: Class<T>): T {
                    return NqAppViewModel(application) as T
                }
            }
        }
    }
}

sealed interface BleEvent {
    data class Log(val message: String) : BleEvent
    data class DevicesFound(val devices: List<BleDeviceItem>) : BleEvent
    data class ConnectionChanged(val connected: Boolean, val name: String?) : BleEvent
    data class FrameReceived(val data: ByteArray) : BleEvent
}

private class NqBleClient(
    private val context: Context,
    private val onEvent: (BleEvent) -> Unit
) {
    private val bluetoothManager =
        context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
    private val adapter: BluetoothAdapter? = bluetoothManager.adapter
    private val scanner get() = adapter?.bluetoothLeScanner
    private val devices = linkedMapOf<String, BleDeviceItem>()

    private var scanCallback: ScanCallback? = null
    private var gatt: BluetoothGatt? = null
    private var writeCharacteristic: BluetoothGattCharacteristic? = null
    private var notifyCharacteristic: BluetoothGattCharacteristic? = null

    @SuppressLint("MissingPermission")
    fun startScan() {
        if (scanCallback != null) return
        scanCallback = object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult) {
                val device = result.device ?: return
                val name = device.name ?: result.scanRecord?.deviceName ?: "Unknown"
                if (!name.contains("RDTSS", true) && !name.contains("NQ", true)) return
                devices[device.address] = BleDeviceItem(device.address, name)
                onEvent(BleEvent.DevicesFound(devices.values.toList()))
            }
        }
        scanner?.startScan(scanCallback)
        onEvent(BleEvent.Log("BLE scan started"))
    }

    @SuppressLint("MissingPermission")
    fun stopScan() {
        scanCallback?.let { scanner?.stopScan(it) }
        scanCallback = null
        onEvent(BleEvent.Log("BLE scan stopped"))
    }

    @SuppressLint("MissingPermission")
    fun connect(address: String) {
        stopScan()
        val device = adapter?.getRemoteDevice(address) ?: return
        gatt?.close()
        gatt = device.connectGatt(context, false, callback, BluetoothDevice.TRANSPORT_LE)
        onEvent(BleEvent.Log("Connecting to ${device.name ?: address}"))
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        gatt?.disconnect()
        gatt?.close()
        gatt = null
        writeCharacteristic = null
        notifyCharacteristic = null
        onEvent(BleEvent.ConnectionChanged(false, null))
    }

    @SuppressLint("MissingPermission")
    fun write(data: ByteArray) {
        val gattRef = gatt ?: return
        val characteristic = writeCharacteristic ?: return
        characteristic.value = data
        characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
        gattRef.writeCharacteristic(characteristic)
    }

    private val callback = object : BluetoothGattCallback() {
        @SuppressLint("MissingPermission")
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            when (newState) {
                BluetoothGatt.STATE_CONNECTED -> {
                    onEvent(BleEvent.ConnectionChanged(true, gatt.device.name))
                    onEvent(BleEvent.Log("GATT connected, discovering services"))
                    gatt.discoverServices()
                }
                BluetoothGatt.STATE_DISCONNECTED -> {
                    onEvent(BleEvent.ConnectionChanged(false, gatt.device.name))
                    onEvent(BleEvent.Log("GATT disconnected"))
                }
            }
        }

        @SuppressLint("MissingPermission")
        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            val service = gatt.getService(NqBleUuid.SERVICE_UUID)
            writeCharacteristic = service?.getCharacteristic(NqBleUuid.WRITE_UUID)
            notifyCharacteristic = service?.getCharacteristic(NqBleUuid.NOTIFY_UUID)
            notifyCharacteristic?.let { characteristic ->
                gatt.setCharacteristicNotification(characteristic, true)
                val descriptor = characteristic.getDescriptor(NqBleUuid.CCCD_UUID)
                descriptor?.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                if (descriptor != null) gatt.writeDescriptor(descriptor)
            }
            onEvent(BleEvent.Log("Service ready: ${service != null}"))
        }

        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic
        ) {
            onEvent(BleEvent.FrameReceived(characteristic.value ?: byteArrayOf()))
        }
    }
}

private object NqBleUuid {
    val SERVICE_UUID: UUID = UUID.fromString("0000FFF0-0000-1000-8000-00805F9B34FB")
    val NOTIFY_UUID: UUID = UUID.fromString("0000FFF1-0000-1000-8000-00805F9B34FB")
    val WRITE_UUID: UUID = UUID.fromString("0000FFF2-0000-1000-8000-00805F9B34FB")
    val CCCD_UUID: UUID = UUID.fromString("00002902-0000-1000-8000-00805F9B34FB")
}

private data class ParsedFrame(
    val command: Int,
    val payload: ByteArray
)

private object NqProtocol {
    private const val HEAD1 = 0x55
    private const val HEAD2 = 0xAA
    private const val TX_VERSION = 0x03

    fun buildStatusRequest(): ByteArray = encode(TX_VERSION, 0x08, byteArrayOf())

    fun buildTimeSync(dateTime: LocalDateTime): ByteArray {
        val weekday = when (dateTime.dayOfWeek.value) {
            7 -> 1
            else -> dateTime.dayOfWeek.value + 1
        }
        return encode(
            TX_VERSION,
            0x1C,
            byteArrayOf(
                1,
                (dateTime.year % 100).toByte(),
                dateTime.monthValue.toByte(),
                dateTime.dayOfMonth.toByte(),
                dateTime.hour.toByte(),
                dateTime.minute.toByte(),
                dateTime.second.toByte(),
                weekday.toByte()
            )
        )
    }

    fun buildDpBooleanWrite(dpId: Int, enabled: Boolean): ByteArray {
        return buildDpWrite(dpId, 0x01, byteArrayOf(if (enabled) 1 else 0))
    }

    fun buildDpByteWrite(dpId: Int, value: Int): ByteArray {
        return buildDpWrite(dpId, 0x01, byteArrayOf(value.toByte()))
    }

    fun buildCustomTiming(minutes: Int, concentration: Int): ByteArray {
        val payload = byteArrayOf(
            ((minutes ushr 8) and 0xFF).toByte(),
            (minutes and 0xFF).toByte(),
            concentration.coerceAtLeast(1).toByte()
        )
        return buildDpWrite(27, 0x00, payload)
    }

    private fun buildDpWrite(dpId: Int, dpType: Int, data: ByteArray): ByteArray {
        val payload = ByteArray(4 + data.size)
        payload[0] = dpId.toByte()
        payload[1] = dpType.toByte()
        payload[2] = ((data.size ushr 8) and 0xFF).toByte()
        payload[3] = (data.size and 0xFF).toByte()
        data.copyInto(payload, 4)
        return encode(TX_VERSION, 0x06, payload)
    }

    fun parseFrame(data: ByteArray): ParsedFrame? {
        if (data.size < 7) return null
        if (data[0].toUnsignedInt() != HEAD1 || data[1].toUnsignedInt() != HEAD2) return null
        val length = (data[4].toUnsignedInt() shl 8) or data[5].toUnsignedInt()
        if (data.size < length + 7) return null
        val checksum = data.copyOfRange(0, length + 6).sumOf { it.toUnsignedInt() } and 0xFF
        if (checksum != data[length + 6].toUnsignedInt()) return null
        return ParsedFrame(
            command = data[3].toUnsignedInt(),
            payload = data.copyOfRange(6, 6 + length)
        )
    }

    private fun encode(version: Int, command: Int, payload: ByteArray): ByteArray {
        val frame = ByteArray(payload.size + 7)
        frame[0] = HEAD1.toByte()
        frame[1] = HEAD2.toByte()
        frame[2] = version.toByte()
        frame[3] = command.toByte()
        frame[4] = ((payload.size ushr 8) and 0xFF).toByte()
        frame[5] = (payload.size and 0xFF).toByte()
        payload.copyInto(frame, 6)
        frame[payload.size + 6] =
            (frame.copyOfRange(0, payload.size + 6).sumOf { it.toUnsignedInt() } and 0xFF).toByte()
        return frame
    }
}

@Composable
private fun NqApp(vm: NqAppViewModel) {
    val ui by vm.uiState.collectAsStateWithLifecycle()
    var timingMinutes by remember { mutableStateOf("60") }
    var timingConcentration by remember { mutableStateOf("1") }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
            .verticalScroll(rememberScrollState()),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text("NQ25015A Mobile", style = MaterialTheme.typography.headlineSmall, fontWeight = FontWeight.Bold)
        Text("Status: ${ui.statusText}")
        Text("Connected: ${ui.connectedName.ifBlank { "None" }}")

        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    Button(onClick = vm::startScan) { Text("Scan") }
                    OutlinedButton(onClick = vm::stopScan) { Text("Stop") }
                    OutlinedButton(onClick = vm::disconnect) { Text("Disconnect") }
                }
                if (ui.devices.isEmpty()) {
                    Text("No devices found yet")
                } else {
                    ui.devices.forEach { device ->
                        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                            Text("${device.name} (${device.address})", modifier = Modifier.weight(1f))
                            TextButton(onClick = { vm.connect(device) }) { Text("Connect") }
                        }
                    }
                }
            }
        }

        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
                Text("Controls", style = MaterialTheme.typography.titleMedium)
                ToggleRow("Aroma", ui.state.aromaOn, vm::setAroma)
                ToggleRow("Fan", ui.state.fanOn, vm::setFan)
                ToggleRow("Light", ui.state.lightOn, vm::setLight)
                ToggleRow("Motor", ui.state.motorOn, vm::setMotor)
                ToggleRow("Key Lock", ui.state.lockOn, vm::setLock)

                Divider()

                Text("Mode: ${if (ui.state.mode == 0) "Simple" else "Professional"}")
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    OutlinedButton(onClick = { vm.setMode(0) }) { Text("Simple") }
                    OutlinedButton(onClick = { vm.setMode(1) }) { Text("Professional") }
                }

                Text("Concentration: ${ui.state.concentration}")
                Slider(
                    value = ui.state.concentration.toFloat(),
                    onValueChange = { vm.setConcentration(it.toInt()) },
                    valueRange = 0f..(ui.state.concentrationCount - 1).coerceAtLeast(2).toFloat(),
                    steps = (ui.state.concentrationCount - 2).coerceAtLeast(0)
                )
            }
        }

        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(10.dp)) {
                Text("Custom Timing", style = MaterialTheme.typography.titleMedium)
                OutlinedTextField(
                    value = timingMinutes,
                    onValueChange = { timingMinutes = it.filter(Char::isDigit) },
                    label = { Text("Minutes") },
                    modifier = Modifier.fillMaxWidth()
                )
                OutlinedTextField(
                    value = timingConcentration,
                    onValueChange = { timingConcentration = it.filter(Char::isDigit) },
                    label = { Text("Concentration (1-based)") },
                    modifier = Modifier.fillMaxWidth()
                )
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    Button(onClick = {
                        vm.setCustomTiming(
                            timingMinutes.toIntOrNull() ?: 60,
                            timingConcentration.toIntOrNull() ?: 1
                        )
                    }) { Text("Send Timing") }
                    OutlinedButton(onClick = vm::syncTime) { Text("Sync Time") }
                    OutlinedButton(onClick = vm::requestAll) { Text("Read All") }
                }
            }
        }

        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
                Text("Runtime Status", style = MaterialTheme.typography.titleMedium)
                Text("Link status: ${ui.state.linkStatus}")
                Text("Work state: ${ui.state.workStatus.state}")
                Text("Remaining sec: ${ui.state.workStatus.remainingSec}")
                Text("Work sec: ${ui.state.workStatus.workSec}")
                Text("Pause sec: ${ui.state.workStatus.pauseSec}")
                Text("Timing minutes: ${ui.state.timingMinutes}")
                Text("Timing concentration: ${ui.state.timingConcentration}")
            }
        }

        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
                Text("Raw Log", style = MaterialTheme.typography.titleMedium)
                ui.logLines.takeLast(40).forEach { line ->
                    Text(line, style = MaterialTheme.typography.bodySmall)
                }
            }
        }
    }
}

@Composable
private fun ToggleRow(label: String, checked: Boolean, onToggle: (Boolean) -> Unit) {
    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
        Text(label)
        Switch(checked = checked, onCheckedChange = onToggle)
    }
}

private fun Byte.toUnsignedInt(): Int = toInt() and 0xFF

private fun ByteArray.toHexString(): String = joinToString(" ") { "%02X".format(it.toUnsignedInt()) }

private fun readU16(data: ByteArray, offset: Int): Int {
    if (offset + 1 >= data.size) return 0
    return (data[offset].toUnsignedInt() shl 8) or data[offset + 1].toUnsignedInt()
}
