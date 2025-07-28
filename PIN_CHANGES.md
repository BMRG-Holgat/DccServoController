# Pin Configuration Changes

## Summary of Changes Made:

### 1. **DCC Input Pin**:
   - **Old**: GPIO 2
   - **New**: GPIO 4
   - **Reason**: Better isolation from boot-sensitive pins

### 2. **Servo Pin Count**:
   - **Old**: 18 servos (GPIO 5-22)
   - **New**: 16 servos (GPIO 5-20)
   - **Reason**: Optimized for available ESP32 PWM channels

### 3. **Pin Mapping**:
   ```
   Servo 0  -> GPIO 5
   Servo 1  -> GPIO 6
   Servo 2  -> GPIO 7
   Servo 3  -> GPIO 8
   Servo 4  -> GPIO 9
   Servo 5  -> GPIO 10
   Servo 6  -> GPIO 11
   Servo 7  -> GPIO 12
   Servo 8  -> GPIO 13
   Servo 9  -> GPIO 14
   Servo 10 -> GPIO 15
   Servo 11 -> GPIO 16
   Servo 12 -> GPIO 17
   Servo 13 -> GPIO 18
   Servo 14 -> GPIO 19
   Servo 15 -> GPIO 20
   ```

### 4. **Files Updated**:
   - `src/config.h` - Updated TOTAL_PINS and DCC_PIN constants
   - `README.md` - Updated documentation and pin assignments
   - `MODULES.md` - Updated module documentation

### 5. **ESP32 PWM Compatibility**:
   All selected pins (GPIO 4-20, 26-27) are PWM-capable on ESP32:
   - GPIO 4: DCC input (interrupt capable)
   - GPIO 5-20: Servo PWM outputs
   - GPIO 26-27: Status LEDs

### 6. **Configuration Commands Updated**:
   Serial commands now accept pin range 5-20:
   ```
   s 5,100,25,0,0    # Configure GPIO 5
   s 20,200,30,1,0   # Configure GPIO 20
   p 5,t             # Control GPIO 5
   ```

This configuration provides optimal use of ESP32 PWM channels while maintaining compatibility with the ESP32Servo library and avoiding boot-sensitive pins.
