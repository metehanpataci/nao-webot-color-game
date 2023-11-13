using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Mail;
using System.Text;
using System.Threading.Tasks;

namespace NaoManagement
{
    public class RobotManager
    {
        private UDPManager udpManager;

        public enum KeyboardCodes{
            WB_COLOR_OFF = 48,
            WB_RED_COLOR = 55,
            WB_GREEN_COLOR,
            WB_BLUE_COLOR,
            WB_KEYBOARD_END = 312,
            WB_KEYBOARD_HOME,
            WB_KEYBOARD_LEFT,
            WB_KEYBOARD_UP,
            WB_KEYBOARD_RIGHT,
            WB_KEYBOARD_DOWN,
            WB_KEYBOARD_PAGEUP = 366,
            WB_KEYBOARD_PAGEDOWN,
            WB_KEYBOARD_NUMPAD_HOME = 375,
            WB_KEYBOARD_NUMPAD_LEFT,
            WB_KEYBOARD_NUMPAD_UP,
            WB_KEYBOARD_NUMPAD_RIGHT,
            WB_KEYBOARD_NUMPAD_DOWN,
            WB_KEYBOARD_NUMPAD_END = 382,
            WB_KEYBOARD_KEY = 65535,
            WB_KEYBOARD_SHIFT = 65536,
            WB_KEYBOARD_CONTROL = 131072,
            WB_KEYBOARD_ALT = 262144,
            WB_HELLO = 33,
            WB_STAND_UP = 81,
            WB_SHOOT = 87,
            WB_TURN_LEFT_40 = 69,
            WB_TURN_RIGHT_40 = 82,
            WB_TURN_AROUND = 84
        }
        public RobotManager() 
        {
            udpManager = new UDPManager();
            
        }

        public void SendCommandToNao(int command)
        {
            udpManager.Send("" + (int)command);
        }
    }
}
