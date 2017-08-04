// - a simple mouse trapper for windows
//
// - keyboard hook function is mostly copy/pase
// - bMapKeys is unused for now
// - code is ugly
// - format is ugly - should use brain instead of clang-format
// - feel free to raise github issues and educate me how to do better

#include <iostream>
#include <thread>
#include <windows.h>

using namespace std;

#define TOGGLE 0x0
#define KILL 0x1

bool bTrap = false;

RECT captureArea;
HWND windowHandle;
MSG msg;
HHOOK keyboardHook;

string message;

void print_message(int nSeverity, string message) {
  switch (nSeverity) {
  case -1:
    cout << message;
    break;
  case 0:
    cout << "[info]\t\t " << message;
    break;
  case 1:
    cout << "[warning]\t " << message;
    break;
  case 2:
    cout << "[error]\t\t " << message;
    break;
  }
}

// thread that tries to keep the focus
void keep_focus_thread(HWND &windowHandle, RECT &captureArea) {
  while (bTrap) {
    BringWindowToTop(windowHandle);
    SetForegroundWindow(windowHandle);
    SetFocus(windowHandle);
    ClipCursor(&captureArea);
    Sleep(50);
  }
}

// process the keyboard event
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode < 0 || nCode != HC_ACTION)
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);

  KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

  if (p->vkCode == VK_TAB && p->flags & LLKHF_ALTDOWN)
    return 1; // disable alt-tab

  if ((p->vkCode == VK_LWIN) || (p->vkCode == VK_RWIN))
    return 1; // disable windows keys

  if (p->vkCode == VK_ESCAPE && p->flags & LLKHF_ALTDOWN)
    return 1; // disable alt-escape

  BOOL bControlKeyDown = GetAsyncKeyState(VK_CONTROL) >>
                         ((sizeof(SHORT) * 8) - 1); // checks ctrl key pressed

  if (p->vkCode == VK_ESCAPE && bControlKeyDown)
    return 1; // disable ctrl-escape

  return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

// the main trap toggle
void toggle_trap(bool &bAggressive, bool &bKeepFocus, bool &bMapkeays,
                 HWND &windowHandle, RECT &captureArea) {

  char window_name[64];

  if (!bTrap) {
    windowHandle = GetForegroundWindow();
    GetWindowRect(windowHandle, &captureArea);
    captureArea.left += 5;
    captureArea.right -= 5;
    captureArea.top += 40;
    captureArea.bottom -= 10;
    ClipCursor(&captureArea);

    if (bAggressive) {
      keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,
                                      GetModuleHandle(NULL), 0);
    }

    if (bKeepFocus) {
      thread t1(keep_focus_thread, ref(windowHandle), ref(captureArea));
      t1.detach();
    }

    bTrap = true;
    GetWindowText(windowHandle, window_name, 64);

    message = "mouse trapped in window " + string(window_name) + "..\n";
    print_message(0, message);
    return;
  }

  if (bTrap) {
    // wait for keep-focus thread to finish, else there might be a race
    // condition
    bTrap = false;
    Sleep(100);
    ClipCursor(NULL);

    if (bAggressive) {
      UnhookWindowsHookEx(keyboardHook);
    }

    message = "mouse released..\n";
    print_message(0, message);
  }
}

void print_help() {
  cout << "usage: trapper [options]" << endl;
  cout << endl;
  cout << "options:" << endl;
  cout << "  --help \t this explanation" << endl;
  cout
      << "  --keep-focus \t periodically tries to make sure we still have focus"
      << endl;
  cout << "  --aggressive \t additionally uses low-level hooks to keep the "
          "mouse trapped"
       << endl;
  cout << endl;
  cout << "notes:" << endl;
  cout << "  - needs elevated rights (i.e. to be run as admin) to trap the "
          "mouse in privileged applications"
       << endl;
  cout << "  - the mouse will be trapped in an area slightly smaller than the "
          "target window"
       << endl;
  cout << "  - aggressive mode also effectively prevents tabbing and other "
          "means of losing focus"
       << endl;
  cout << "  - options can be combined" << endl;
}

int main(int argc, char *argv[]) {
  bool bKeepFocus = false;
  bool bAggressive = false;
  bool bMapKeys = false;

  // check and process command line options
  if (argc > 1) {
    if ((argc == 2) && (string(argv[1]) == "--help")) {
      print_help();
      return 0;
    }

    for (int i = 1; i < argc; ++i) {
      if (string(argv[i]) == "--aggressive") {
        bAggressive = true;
        continue;
      }

      if (string(argv[i]) == "--keep-focus") {
        bKeepFocus = true;
        continue;
      }

      if (string(argv[i]) == "--map-keys") {
        bMapKeys = true;
        continue;
      }

      else {
        message = "invalid command line option \"" + string(argv[i]) + "\"\n";
        print_message(1, message);
        print_help();
        return 1;
      }
    }
  }

  if (!RegisterHotKey(0, KILL, 0, 0x23)) {
    message = "cannot register hotkeys..exiting.\n";
    print_message(2, message);
    return 1;
  }

  if (!RegisterHotKey(0, TOGGLE, 0, 0x24)) {
    message = "cannot register hotkeys..exiting.\n";
    print_message(2, message);
    return 1;
  }

  cout << "usage: click target window, HOME to toggle trap, END to kill" << endl
       << endl;

  message =
      "make sure you have admin rights to trap in privileged applications\n";
  print_message(0, message);

  message = "trapping: yes, keep focus: ";
  print_message(0, message);

  if (bKeepFocus) {
    message = "yes, ";
  } else {
    message = "no, ";
  }
  print_message(-1, message);

  message = "aggressive: ";
  print_message(-1, message);

  if (bAggressive) {
    message = "yes, ready to trap..\n";
  } else {
    message = "no, ready to trap..\n";
  }
  print_message(-1, message);

  // process hotkey events
  while (GetMessage(&msg, NULL, 0, 0) != 0) {

    if (msg.wParam == TOGGLE) {
      toggle_trap(bAggressive, bKeepFocus, bMapKeys, windowHandle, captureArea);
      continue;
    }

    if (msg.wParam == KILL) {

      if (bTrap) {
        toggle_trap(bAggressive, bKeepFocus, bMapKeys, windowHandle,
                    captureArea);
      }

      message = "cleaning up..";
      print_message(0, message);

      Sleep(500);

      message = "done.\n";
      print_message(-1, message);
      return 0;
    }
  }

  return 0;
}
