// ================================================================
//  LINE FOLLOWER — ANUGAMI BUILD VX (RACE OPTIMIZED)
// ================================================================

// ── PINS ─────────────────────────────────────────────────────────
#define ENA 6
#define IN1 9
#define IN2 8
#define ENB 5
#define IN3 7
#define IN4 4

const uint8_t SENSOR_PINS[5] = {11, 10, 12, 13, A0};

// ── SPEED TUNING (HIGH SPEED) ───────────────────────────────────
const int BASE_SPEED   = 160;   
const int GENTLE_INNER = 130;   
const int MEDIUM_INNER = 90;    
const int SHARP_INNER  = -160;  

// ── LAST MOTOR MEMORY ────────────────────────────────────────────
int lastL = 0;
int lastR = 0;

// ================================================================
//  MOTORS
// ================================================================

void setMotors(int L, int R) {
  L = constrain(L, -255, 255);
  R = constrain(R, -255, 255);

  // LEFT
  if (L >= 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    L = -L;
  }
  analogWrite(ENA, L);

  // RIGHT
  if (R >= 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    R = -R;
  }
  analogWrite(ENB, R);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ================================================================
//  SETUP
// ================================================================

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  for (int i = 0; i < 5; i++) {
    pinMode(SENSOR_PINS[i], INPUT);
  }
  
  // 2-second delay gives you time to place the robot and move your hands away
  delay(2000); 
}

// ================================================================
//  ACTION LOGIC
// ================================================================

void getAction(int pattern, int &L, int &R) {
  switch (pattern) {
    // ── STRAIGHT ────────────────────────────────
    case 0b11011:
    case 0b00100:
    case 0b01110:
      L = BASE_SPEED;
      R = BASE_SPEED;
      break;

    // ── LEFT TURNS ──────────────────────────────
    case 0b11101:
    case 0b11001:
      L = BASE_SPEED;
      R = GENTLE_INNER;
      break;

    case 0b11100:
      L = BASE_SPEED;
      R = MEDIUM_INNER;
      break;

    case 0b11110:
      L = BASE_SPEED;
      R = SHARP_INNER;
      break;

    // ── RIGHT TURNS ─────────────────────────────
    case 0b10111:
    case 0b10011:
      L = GENTLE_INNER;
      R = BASE_SPEED;
      break;

    case 0b00111:
      L = MEDIUM_INNER;
      R = BASE_SPEED;
      break;

    case 0b01111:
      L = SHARP_INNER;
      R = BASE_SPEED;
      break;

    // ── DEFAULT AUTO BALANCE ─────────────────────
    default: {
      int leftWeight  = ((pattern >> 4) & 1) * 2 + ((pattern >> 3) & 1);
      int rightWeight = ((pattern >> 1) & 1) + ((pattern >> 0) & 1) * 2;

      if (leftWeight > rightWeight) {
        L = GENTLE_INNER;
        R = BASE_SPEED;
      } else if (rightWeight > leftWeight) {
        L = BASE_SPEED;
        R = GENTLE_INNER;
      } else {
        L = BASE_SPEED;
        R = BASE_SPEED;
      }
      break;
    }
  }
}

// ================================================================
//  LOOP
// ================================================================

void loop() {
  int s1 = digitalRead(SENSOR_PINS[0]);
  int s2 = digitalRead(SENSOR_PINS[1]);
  int s3 = digitalRead(SENSOR_PINS[2]);
  int s4 = digitalRead(SENSOR_PINS[3]);
  int s5 = digitalRead(SENSOR_PINS[4]);

  int pattern = (s1 << 4) | (s2 << 3) | (s3 << 2) | (s4 << 1) | s5;

  // ===== STOP (e.g., T-Junction or All Black/White depending on surface) =====
  if (pattern == 0b00000) {
    stopMotors();
    return;
  }

  // ===== RECOVERY =====
    // 0b11111 means all sensors are reading WHITE (assuming 0 is black)
    if (pattern == 0b11111) {
      setMotors(-80, -80);
      /*
        if (lastL == lastR) {
          // Exception: It was going straight. Reverse straight back.
          setMotors(-130, -130);
        } 
        else if (lastL > lastR) {
          // Last known action was a right turn. Spin right.
          setMotors(140, -140); 
        } 
        else {
          // Last known action was a left turn. Spin left.
          setMotors(-140, 140); 
        }
        */
        return; 
    }

  // ===== NORMAL =====
  int L, R;
  getAction(pattern, L, R);

  setMotors(L, R);

  lastL = L;
  lastR = R;
}