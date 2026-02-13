# USB HID Reference

This document serves as a complete technical specification for implementing the firmware's USB interface. It details the
dual-mode architecture, exact USB descriptors, and HID report structures required for compatibility.

---

## Dual-Mode Architecture

The device implements a dual-mode USB architecture to satisfy two distinct requirements:

1. **Programming Mode**: Driverless communication via WebHID (requires Raw HID / Vendor Page).
2. **Keyboard Mode**: Standard keyboard functionality (requires Boot Protocol HID).

### Mode Switching

The device determines its mode at startup and exposes different USB descriptors accordingly. Transitioning between modes
requires a device reset (soft reboot) to force re-enumeration by the host.

| Feature              | Programming Mode          | Keyboard Mode            |
|----------------------|---------------------------|--------------------------|
| **Interface Class**  | `0x03` (HID)              | `0x03` (HID)             |
| **Subclass**         | `0x00` (None)             | `0x01` (Boot Interface)  |
| **Protocol**         | `0x00` (None)             | `0x01` (Keyboard)        |
| **Usage Page**       | `0xFF00` (Vendor Defined) | `0x01` (Generic Desktop) |
| **Usage**            | `0x01` (Vendor Usage 1)   | `0x06` (Keyboard)        |
| **Primary Endpoint** | EP0 (Control)             | EP1 (Interrupt IN)       |

---

## USB Descriptors

The firmware must generate descriptors dynamically based on the active mode.

### Device Descriptor

Common to both modes.

| Field             | Value    | Description          |
|-------------------|----------|----------------------|
| **idVendor**      | `0x16C0` | Shared VID (Voti.nl) |
| **idProduct**     | `0x27DB` | Shared PID           |
| **bcdDevice**     | `0x0001` | v1.00                |
| **iManufacturer** | `1`      | "TinyKB"             |
| **iProduct**      | `2`      | "TinyKB"             |

### 1. Programming Mode Descriptors

Exposes a Raw HID interface for bidirectional 32-byte transfers.

**Interface Descriptor:**

- **bInterfaceClass**: `0x03`
- **bInterfaceSubClass**: `0x00`
- **bInterfaceProtocol**: `0x00`

**HID Report Descriptor (29 bytes):**

```c
0x06, 0x00, 0xFF,   /* USAGE_PAGE (Vendor Defined 0xFF00) */
0x09, 0x01,         /* USAGE (Vendor Usage 1)             */
0xA1, 0x01,         /* COLLECTION (Application)           */
                    /*                                    */
                    /* Input Report (Device -> Host)      */
0x15, 0x00,         /*   LOGICAL_MINIMUM (0)              */
0x26, 0xFF, 0x00,   /*   LOGICAL_MAXIMUM (255)            */
0x75, 0x08,         /*   REPORT_SIZE (8) - 1 byte fields  */
0x95, 0x20,         /*   REPORT_COUNT (32)                */
0x09, 0x01,         /*   USAGE (Vendor Usage 1)           */
0x81, 0x02,         /*   INPUT (Data,Var,Abs)             */
                    /*                                    */
                    /* Output Report (Host -> Device)     */
0x09, 0x01,         /*   USAGE (Vendor Usage 1)           */
0x91, 0x02,         /*   OUTPUT (Data,Var,Abs)            */
                    /*                                    */
                    /* Feature Report (Bidirectional)     */
0x09, 0x01,         /*   USAGE (Vendor Usage 1)           */
0xB1, 0x02,         /*   FEATURE (Data,Var,Abs)           */
                    /*                                    */
0xC0                /* END_COLLECTION                     */
```

### 2. Keyboard Mode Descriptors

Exposes a standard Boot Protocol Keyboard interface.

**Interface Descriptor:**

- **bInterfaceClass**: `0x03`
- **bInterfaceSubClass**: `0x01`
- **bInterfaceProtocol**: `0x01`

**HID Report Descriptor (63 bytes):**

```c
0x05, 0x01,         /* USAGE_PAGE (Generic Desktop)       */
0x09, 0x06,         /* USAGE (Keyboard)                   */
0xA1, 0x01,         /* COLLECTION (Application)           */
                    /*                                    */
                    /* Modifier Byte                      */
0x05, 0x07,         /*   USAGE_PAGE (Keyboard)            */
0x19, 0xE0,         /*   USAGE_MINIMUM (Left Ctrl)        */
0x29, 0xE7,         /*   USAGE_MAXIMUM (Right GUI)        */
0x15, 0x00,         /*   LOGICAL_MINIMUM (0)              */
0x25, 0x01,         /*   LOGICAL_MAXIMUM (1)              */
0x75, 0x01,         /*   REPORT_SIZE (1)                  */
0x95, 0x08,         /*   REPORT_COUNT (8)                 */
0x81, 0x02,         /*   INPUT (Data,Var,Abs)             */
                    /*                                    */
                    /* Reserved Byte                      */
0x95, 0x01,         /*   REPORT_COUNT (1)                 */
0x75, 0x08,         /*   REPORT_SIZE (8)                  */
0x81, 0x03,         /*   INPUT (Cnst,Var,Abs)             */
                    /*                                    */
                    /* LEDs (Output)                      */
0x95, 0x05,         /*   REPORT_COUNT (5)                 */
0x75, 0x01,         /*   REPORT_SIZE (1)                  */
0x05, 0x08,         /*   USAGE_PAGE (LEDs)                */
0x19, 0x01,         /*   USAGE_MINIMUM (Num Lock)         */
0x29, 0x05,         /*   USAGE_MAXIMUM (Kana)             */
0x91, 0x02,         /*   OUTPUT (Data,Var,Abs)            */
0x95, 0x01,         /*   REPORT_COUNT (1)                 */
0x75, 0x03,         /*   REPORT_SIZE (3)                  */
0x91, 0x03,         /*   OUTPUT (Cnst,Var,Abs)            */
                    /*                                    */
                    /* Key Arrays (6 Bytes)               */
0x95, 0x06,         /*   REPORT_COUNT (6)                 */
0x75, 0x08,         /*   REPORT_SIZE (8)                  */
0x15, 0x00,         /*   LOGICAL_MINIMUM (0)              */
0x25, 0x65,         /*   LOGICAL_MAXIMUM (101)            */
0x05, 0x07,         /*   USAGE_PAGE (Keyboard)            */
0x19, 0x00,         /*   USAGE_MINIMUM (0)                */
0x29, 0x65,         /*   USAGE_MAXIMUM (101)              */
0x81, 0x00,         /*   INPUT (Data,Ary,Abs)             */
                    /*                                    */
0xC0                /* END_COLLECTION                     */
```

---

## Control Request Handling

To support the dual-mode architecture, the USB stack must correctly handle specific Control Endpoint (EP0) requests.

### 1. GET_REPORT (0x01)

Used by the host to read data from the device.

**Setup Packet:**
| Offset | Field | Value | Description |
|--------|-----------------|----------------|--------------------------------|
| 0 | `bmRequestType` | `0xA1`         | Class, Interface, Device->Host |
| 1 | `bRequest`      | `0x01`         | GET_REPORT |
| 2 | `wValueL`       | `0x01` - `0x03`| Report Type (Input/Output/Feature) |
| 3 | `wValueH`       | `0x00`         | Report ID (0)                  |
| 4 | `wIndex`        | `0x00`         | Interface Number |
| 6 | `wLength`       | `N`            | Requested Length |

**Implementation Logic:**

- **Programming Mode**: Used to retrieve command responses (Feature Report).
- **Keyboard Mode**: Rarely used (Input Report via EP1 usually).

### 2. SET_REPORT (0x09)

Used by the host to send data to the device.

**Setup Packet:**
| Offset | Field | Value | Description |
|--------|-----------------|----------------|--------------------------------|
| 0 | `bmRequestType` | `0x21`         | Class, Interface, Host->Device |
| 1 | `bRequest`      | `0x09`         | SET_REPORT |
| 2 | `wValueL`       | `0x02` - `0x03`| Report Type (Output/Feature)   |
| 3 | `wValueH`       | `0x00`         | Report ID (0)                  |
| 4 | `wIndex`        | `0x00`         | Interface Number |
| 6 | `wLength`       | `N`            | Payload Length |

**Implementation Logic:**

- **Programming Mode**: Used to send commands (Output Report). Payload is 32 bytes.
- **Keyboard Mode**: Used to update LEDs (Output Report). Payload is 1 byte.

### 3. SET_IDLE (0x0A)

Used by the host to set the idle rate for Input Reports.

**Setup Packet:**
| Offset | Field | Value | Description |
|--------|-----------------|----------------|--------------------------------|
| 0 | `bmRequestType` | `0x21`         | Class, Interface, Host->Device |
| 1 | `bRequest`      | `0x0A`         | SET_IDLE |
| 2 | `wValueL`       | `0x00` - `0xFF`| Idle Rate (4ms units)          |
| 3 | `wValueH`       | `0x00`         | Report ID (0)                  |
| 4 | `wIndex`        | `0x00`         | Interface Number |

---

## Endpoint Configuration

### Programming Mode (Raw HID)

Uses **Control Transfers (Endpoint 0)** for all data. This design choice bypasses the 8-byte limit of Low-Speed
Interrupt endpoints, allowing for full 32-byte command/response payloads.

| Endpoint | Type      | Direction | Size     | Usage                            |
|----------|-----------|-----------|----------|----------------------------------|
| **EP0**  | Control   | Bi-Dir    | 8 Bytes* | Command & Response (Data)        |
| **EP1**  | Interrupt | IN        | 8 Bytes  | Unused (Host Compatibility only) |

*Note: While EP1 is present in the descriptors to satisfy OS HID requirements, it is not used for data. EP0 physical
packet size is 8 bytes, but the USB stack handles multi-packet transactions for 32-byte reports transparently.*

### Keyboard Mode

Uses **Interrupt Transfers (Endpoint 1)** for low-latency keystroke delivery.

| Endpoint | Type      | Direction | Size    | Polling | Usage               |
|----------|-----------|-----------|---------|---------|---------------------|
| **EP1**  | Interrupt | IN        | 8 Bytes | 10ms    | Keystroke reporting |

---

## References

- [Device Class Definition for HID 1.11](https://www.usb.org/sites/default/files/documents/hid1_11.pdf)
- [HID Usage Tables 1.12](https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf)
