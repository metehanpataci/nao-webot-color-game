using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NaoManagement
{
    public class GameStatus
    {
        public const int CG_NOT_STARTED = 0;
        public const int CG_STARTED = 1;
        public const int CG_END = 2;

        public int Status { get; set; }

        public int currGameWinner { get; set; }

        public int Winner { get; set; }

        public GameStatus() 
        {
            Status = CG_NOT_STARTED;
            Winner = ColorGamePlayer.PLAYER_PARTICIPANT;
            currGameWinner = ColorGamePlayer.PLAYER_PARTICIPANT;
        }
    }
}
