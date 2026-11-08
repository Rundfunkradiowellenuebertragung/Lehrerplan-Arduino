
#include <LiquidCrystal.h>

// LCD im 4-Bit-Modus
// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

// ---------------- TASTER ----------------

const byte BUTTON_UP = 2;
const byte BUTTON_DOWN = 3;
const byte BUTTON_OK = 4;

// Entprellung
unsigned long lastButtonChange = 0;
const unsigned long debounceDelay = 40;

bool lastUpState = HIGH;
bool lastDownState = HIGH;
bool lastOkState = HIGH;

// ---------------- MENUES ----------------

#define NONE 0
#define UP 1
#define DOWN 2
#define SELECT 3

enum Screen {
  MAIN_MENU,
  TEACHER_MENU,
  DAY_MENU,
  PLAN_VIEW
};

Screen screen = MAIN_MENU;

byte teacherIndex = 0;
byte dayIndex = 0;
byte planPosition = 0;

// ---------------- STUNDENPLAN-DATEN ----------------

struct Lesson {
  byte hour;
  const char* subject;
  const char* room;
};

// Montag bis Freitag

const Lesson mueller[5][5] = {
  {
    {8, "Mathe", "Raum 102"},
    {9, "Mathe", "Raum 102"},
    {10, "Freistunde", ""},
    {11, "Physik", "Raum 204"},
    {12, "Physik", "Raum 204"}
  },
  {
    {8, "Deutsch", "Raum 105"},
    {9, "Englisch", "Raum 105"},
    {10, "Chemie", "Raum 203"},
    {11, "Chemie", "Raum 203"},
    {12, "Geschichte", "Raum 108"}
  },
  {
    {8, "Physik", "Raum 204"},
    {9, "Physik", "Raum 204"},
    {10, "Mathe", "Raum 102"},
    {11, "Mathe", "Raum 102"},
    {12, "Sport", "Sporthalle"}
  },
  {
    {8, "Biologie", "Raum 205"},
    {9, "Biologie", "Raum 205"},
    {10, "Deutsch", "Raum 105"},
    {11, "Englisch", "Raum 105"},
    {12, "Freistunde", ""}
  },
  {
    {8, "Chemie", "Raum 203"},
    {9, "Chemie", "Raum 203"},
    {10, "Mathe", "Raum 102"},
    {11, "Geschichte", "Raum 108"},
    {0, "", ""}
  }
};

const Lesson schmidt[5][5] = {
  {
    {8, "Deutsch", "Raum 105"},
    {9, "Englisch", "Raum 105"},
    {10, "Biologie", "Raum 205"},
    {11, "Biologie", "Raum 205"},
    {12, "Freistunde", ""}
  },
  {
    {8, "Mathe", "Raum 103"},
    {9, "Mathe", "Raum 103"},
    {10, "Deutsch", "Raum 105"},
    {11, "Geschichte", "Raum 108"},
    {12, "Englisch", "Raum 105"}
  },
  {
    {8, "Chemie", "Raum 203"},
    {9, "Chemie", "Raum 203"},
    {10, "Physik", "Raum 204"},
    {11, "Physik", "Raum 205"},
    {12, "Deutsch", "Raum 105"}
  },
  {
    {8, "Englisch", "Raum 105"},
    {9, "Englisch", "Raum 105"},
    {10, "Mathe", "Raum 103"},
    {11, "Biologie", "Raum 205"},
    {12, "Biologie", "Raum 205"}
  },
  {
    {8, "Geschichte", "Raum 108"},
    {9, "Deutsch", "Raum 105"},
    {10, "Physik", "Raum 204"},
    {11, "Mathe", "Raum 103"},
    {0, "", ""}
  }
};

const byte lessonCounts[2][5] = {
  {5, 5, 5, 5, 4},
  {5, 5, 5, 5, 4}
};

const char* teacherNames[2] = {
  "Herr Mueller",
  "Frau Schmidt"
};

const char* dayNames[5] = {
  "Montag",
  "Dienstag",
  "Mittwoch",
  "Donnerstag",
  "Freitag"
};

// ---------------- LCD-ANZEIGE ----------------

void show(const char* row1, const char* row2) {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(row1);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(row2);
}

// ---------------- TASTER ABFRAGEN ----------------

int readButtons() {

  if (digitalRead(BUTTON_UP) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_UP) == LOW) {
      while (digitalRead(BUTTON_UP) == LOW) {}
      return UP;
    }
  }

  if (digitalRead(BUTTON_DOWN) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_DOWN) == LOW) {
      while (digitalRead(BUTTON_DOWN) == LOW) {}
      return DOWN;
    }
  }

  if (digitalRead(BUTTON_OK) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_OK) == LOW) {
      while (digitalRead(BUTTON_OK) == LOW) {}
      return SELECT;
    }
  }

  return NONE;
}

// ---------------- MENUE-ANZEIGEN ----------------

void showMainMenu() {
  show("> Lehrerplaene", "OK = Oeffnen");
}

void showTeacherMenu() {
  if (teacherIndex == 0) {
    show("> Herr Mueller", "  Frau Schmidt");
  }
  else {
    show("  Herr Mueller", "> Frau Schmidt");
  }
}

void showDayMenu() {
  char row1[17];

  snprintf(row1, sizeof(row1), "> %s", dayNames[dayIndex]);

  show(row1, "^ = Zurueck");
}

// ---------------- STUNDENPLAN ----------------

void showPlan() {

  byte count = lessonCounts[teacherIndex][dayIndex];

  if (planPosition >= count) {
    planPosition = 0;
  }

  const Lesson& lesson =
    (teacherIndex == 0)
    ? mueller[dayIndex][planPosition]
    : schmidt[dayIndex][planPosition];

  char row1[17];
  char row2[17];

  snprintf(row1, sizeof(row1), "%02d:00 %s",
           lesson.hour, lesson.subject);

  snprintf(row2, sizeof(row2), "%s %d/%d",
           lesson.room[0] ? lesson.room : "Pause",
           planPosition + 1, count);

  show(row1, row2);
}

// ---------------- SETUP ----------------

void setup() {

  // Taster mit internem Pullup-Widerstand
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_OK, INPUT_PULLUP);

  lcd.begin(16, 2);

  // Startbildschirm
  show("Lehrer Website", "JGG");

  delay(2000);

  // Direkt ins Hauptmenue
  screen = MAIN_MENU;
  showMainMenu();
}

// ---------------- HAUPTPROGRAMM ----------------

void loop() {

  int action = readButtons();

  bool up   = action == UP;
  bool down = action == DOWN;
  bool ok   = action == SELECT;

  // NACH OBEN = EINE MENUEEBENE ZURUECK

  if (up) {

    switch (screen) {

      case MAIN_MENU:
        break;

      case TEACHER_MENU:
        screen = MAIN_MENU;
        showMainMenu();
        break;

      case DAY_MENU:
        screen = TEACHER_MENU;
        showTeacherMenu();
        break;

      case PLAN_VIEW:
        screen = DAY_MENU;
        showDayMenu();
        break;
    }
  }

  // NACH UNTEN = NAECHSTER MENUEPUNKT

  if (down) {

    if (screen == TEACHER_MENU) {

      teacherIndex = (teacherIndex + 1) % 2;

      showTeacherMenu();
    }

    else if (screen == DAY_MENU) {

      dayIndex = (dayIndex + 1) % 5;

      showDayMenu();
    }

    else if (screen == PLAN_VIEW) {

      byte count = lessonCounts[teacherIndex][dayIndex];

      planPosition = (planPosition + 1) % count;

      showPlan();
    }
  }

  // OK = AUSWAEHLEN

  if (ok) {

    switch (screen) {

      case MAIN_MENU:

        screen = TEACHER_MENU;
        teacherIndex = 0;

        showTeacherMenu();
        break;

      case TEACHER_MENU:

        screen = DAY_MENU;
        dayIndex = 0;

        showDayMenu();
        break;

      case DAY_MENU:

        screen = PLAN_VIEW;
        planPosition = 0;

        showPlan();
        break;

      case PLAN_VIEW:
        break;
    }
  }
}
