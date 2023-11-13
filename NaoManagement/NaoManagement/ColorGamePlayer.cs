using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NaoManagement
{

    public class ColorGamePlayer
    {

        public const int PLAYER_PARTICIPANT = 0;
        public const int PLAYER_ROBOT = 1;

        public const int MAX_PLAYER_COUNT = 2;

        public int SelectedColor {  get;  set; }

        public int PlayerType { get; set; }


        public int Score { get; set; }
    

        public ColorGamePlayer() 
        {
            Score = 0;
            SelectedColor = ColorGame.CG_COLOR_RED; ;
        }


        public void Reset() 
        {
            Score = 0;
            SelectedColor = ColorGame.CG_COLOR_RED;
        }


        public void RandomizeColor()
        {
            Random randVal = new Random();
            SelectedColor = randVal.Next(ColorGame.CG_COLOR_CNT);
        }

        public void Win() 
        {
            Score = Score + 1;
        }


    }
}
