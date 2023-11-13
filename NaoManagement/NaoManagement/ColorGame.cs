using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace NaoManagement
{
    public class ColorGame
    {
        public const int CG_COLOR_RED = 0;
        public const int CG_COLOR_GREEN = 1;
        public const int CG_COLOR_BLUE = 2;
        public const int CG_COLOR_CNT = 3;

        private int MAX_SCORE = 3;

        private GameStatus gameStatus;

        public ColorGamePlayer participant;

        public ColorGamePlayer robot;


        public ColorGame()
        {
            participant = new ColorGamePlayer();
            robot = new ColorGamePlayer();
            gameStatus = new GameStatus();
        }

        public void ResetGame()
        {
            participant.Reset();
            robot.Reset();
            gameStatus = new GameStatus();
        }

        public GameStatus Play(int inColor)
        {
            if (participant.Score == MAX_SCORE || robot.Score == MAX_SCORE || gameStatus.Status == GameStatus.CG_END)
                ResetGame();

            gameStatus.Status = GameStatus.CG_STARTED;

            participant.SelectedColor = inColor;

            robot.RandomizeColor();

            if (participant.SelectedColor == robot.SelectedColor)
            {
                participant.Win();
                gameStatus.currGameWinner = ColorGamePlayer.PLAYER_PARTICIPANT;
            }
            else 
            {
                robot.Win();
                gameStatus.currGameWinner = ColorGamePlayer.PLAYER_ROBOT;
            }

            if (participant.Score == MAX_SCORE)
            {
                gameStatus.Status = GameStatus.CG_END;
                gameStatus.Winner = ColorGamePlayer.PLAYER_PARTICIPANT;
            }
            else if (robot.Score == MAX_SCORE) 
            {
                gameStatus.Status = GameStatus.CG_END;
                gameStatus.Winner = ColorGamePlayer.PLAYER_ROBOT;
            }

            return gameStatus;

        }

        public Boolean isDues() 
        {
            return participant.Score == robot.Score ? true : false;
        }


        public int GetColorCode(string inColorStr)
        {
            switch (inColorStr)
            {
                case "red":
                    return CG_COLOR_RED;
                case "green":
                    return CG_COLOR_GREEN;
                case "blue":
                    return CG_COLOR_BLUE;
            }

            return CG_COLOR_CNT;
        }


    }
}
