using System;
using System.Collections.Generic;
using System.Linq;
using System.Speech.Recognition;
using System.Speech.Synthesis;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace NaoManagement
{
    /// <summary> 
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
		public RobotManager robotManager = null ;
		private SpeechSynthesizer synthesizer = new SpeechSynthesizer();
		private ColorGame colorGame = new ColorGame();

		private GameStatus gameStatus;

		private String []robotColors ={ "red", "green", "blue" };


		public MainWindow()
        {
            InitializeComponent();
			initGUI();

			List<RecognizerInfo> engines = SpeechRecognitionEngine.InstalledRecognizers().ToList();

			for (int i = 0; i < engines.Count; i++)
			{
				Console.WriteLine(i +" - "+engines[i].Culture);
			}
			speechFunc();

			robotManager = new RobotManager();
			synthesizer.Volume = 100;  // 0...100
			synthesizer.Rate = -2;     // -10...10

		}
		private static int i = 0;
		public void line_mouseDown(Object sender, MouseButtonEventArgs args) 
		{
			Console.WriteLine("line clicked..");
			
			lblDemo.Content = "Clicked "+i++;
		}

		private void initGUI() 
		{
			this.KeyDown += new KeyEventHandler(OnButtonKeyDown);
		
			

			/*
			Line mainLine = new Line();
			mainLine.X1 = 20;
			mainLine.X2 = 300;
			mainLine.Y1 = 400;
			mainLine.Y2 = 400;
			mainLine.StrokeThickness = 30;
			mainLine.MouseDown += new MouseButtonEventHandler(line_mouseDown);
			mainLine.Stroke = Brushes.Blue;

			mainStackPanel.Children.Add(mainLine);
			*/
		}

		private SpeechRecognitionEngine speechRecognizer = new SpeechRecognitionEngine(new System.Globalization.CultureInfo("en-US"));

		public void speechFunc()
		{
			//InitializeComponent();
			speechRecognizer.SpeechRecognized += speechRecognizer_SpeechRecognized;

			GrammarBuilder grammarBuilder = new GrammarBuilder();
			grammarBuilder.Culture = speechRecognizer.RecognizerInfo.Culture;


			Choices commandChoices = new Choices("color","move","let's","turn","hi","stand");
			grammarBuilder.Append(commandChoices);

			Choices valueChoices = new Choices();
			valueChoices.Add(robotColors);
			valueChoices.Add("forward", "backward", "left","right");
			valueChoices.Add("play");
			valueChoices.Add("left","right","around");
			valueChoices.Add("nao");
			valueChoices.Add("up");
			grammarBuilder.Append(valueChoices);
			
			speechRecognizer.LoadGrammar(new Grammar(grammarBuilder));
			speechRecognizer.SetInputToDefaultAudioDevice();
			speechRecognizer.RecognizeAsync(RecognizeMode.Multiple);


		}

		private void btnToggleListening_Click(object sender, RoutedEventArgs e)
		{
			/*
			if (btnToggleListening.IsChecked == true)
			{
				EnableSpeechRecog(true);
				btnToggleListening.Content = "Listen Enabled";
			}
			else
			{
				EnableSpeechRecog(false);
				btnToggleListening.Content = "Listen Disabled";
			}
			*/
		}


		private void EnableSpeechRecog(bool isEnabled) 
		{
			speechRecognizer.RecognizeAsyncStop();
			
			if (isEnabled)
				speechRecognizer.RecognizeAsync(RecognizeMode.Multiple);
			

		}


		private void speechRecognizer_SpeechRecognized(object sender, SpeechRecognizedEventArgs e)
		{
			Console.WriteLine("Rec Func exec..");
			lblDemo.Content = e.Result.Text;
			if (e.Result.Words.Count == 2)
			{
				string command = e.Result.Words[0].Text.ToLower();
				string value = e.Result.Words[1].Text.ToLower();
				//EnableSpeechRecog(false);
				
				switch (command)
				{
					case "color":
						//lblDemo.Foreground = new SolidColorBrush((Color)ColorConverter.ConvertFromString(value));
						if (!robotColors.Contains(value))
						{
							return;
						}
						

						gameStatus = colorGame.Play(colorGame.GetColorCode(value));

						int robotColor = colorGame.robot.SelectedColor;
						int robotColorCode = (int)RobotManager.KeyboardCodes.WB_COLOR_OFF;
						String robotColorStr = "nothing";
						switch (robotColor) 
						{
							case ColorGame.CG_COLOR_RED:
								robotColorCode = (int)RobotManager.KeyboardCodes.WB_RED_COLOR;
								robotColorStr = "red";
								robotScoreTextBox.Background = Brushes.Red;
								break;
							case ColorGame.CG_COLOR_GREEN:
								robotColorCode = (int)RobotManager.KeyboardCodes.WB_GREEN_COLOR;
								robotColorStr = "green";
								robotScoreTextBox.Background = Brushes.Green;
								break;
							case ColorGame.CG_COLOR_BLUE:
								robotColorCode = (int)RobotManager.KeyboardCodes.WB_BLUE_COLOR;
								robotColorStr = "blue";
								robotScoreTextBox.Background = Brushes.Blue;
								break;
						}

						switch (value) 
						{
							case "red":
								userScoreTextBox.Background = Brushes.Red;
								break;

							case "green":
								userScoreTextBox.Background = Brushes.Green;
								break;
							case "blue":
								userScoreTextBox.Background = Brushes.Blue;
								break;
								
						}

						


						robotManager.SendCommandToNao((int)robotColorCode);
						SetScores();

						if (gameStatus.Status == GameStatus.CG_END)
						{
							switch (gameStatus.Winner) 
							{
								case ColorGamePlayer.PLAYER_PARTICIPANT:
									winnerLabel.Content = "Participant";
									synthesizer.SpeakAsync("Congratulations You win Game ended.");
									robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_HELLO);
									break;
								case ColorGamePlayer.PLAYER_ROBOT:
									winnerLabel.Content = "Nao";
									synthesizer.SpeakAsync("My color is " + robotColorStr);
									synthesizer.SpeakAsync("I win");
									robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_SHOOT);
									break;
							}
						}
						else 
						{
							if (colorGame.isDues())
							{
								synthesizer.SpeakAsync("Dues");
							}
							else 
							{
								switch (gameStatus.currGameWinner)
								{
									case ColorGamePlayer.PLAYER_PARTICIPANT:
										
										synthesizer.SpeakAsync("Congratulations. Correct");
										robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_HELLO);
										break;
									case ColorGamePlayer.PLAYER_ROBOT:
										
										
										robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_SHOOT);
										synthesizer.SpeakAsync("My color is " + robotColorStr);
										break;
								}
							}

						}

						break;

					case "move":
						switch (value)
						{
							case "forward":
								// Synchronous

								// Asynchronous
								robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_UP);
								break;
							case "backward":
								robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_DOWN);
								break;
							case "left":
								robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_LEFT);
								break;
							case "right":
								robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_RIGHT);
								break;
						}
						break;
					case "let's":
						switch (value)
						{
							case "play":

								StartGame();

								// Synchronous
								BeginGame();
								// Asynchronous
								//robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_UP);
								break;
						}
						break;
					case "turn":
						switch (value)
						{
							case "left":
								// Synchronous
								synthesizer.SpeakAsync("Okay");
								robotManager.SendCommandToNao(((int)RobotManager.KeyboardCodes.WB_KEYBOARD_LEFT) | ((int)RobotManager.KeyboardCodes.WB_KEYBOARD_SHIFT));
								break;
							case "right":
								synthesizer.SpeakAsync("Okay");
								robotManager.SendCommandToNao(((int)RobotManager.KeyboardCodes.WB_KEYBOARD_RIGHT) | ((int)RobotManager.KeyboardCodes.WB_KEYBOARD_SHIFT));
								break;
							case "around":
								synthesizer.SpeakAsync("Okay");
								robotManager.SendCommandToNao(((int)RobotManager.KeyboardCodes.WB_TURN_AROUND));
								break;
						}
						break;
					case "hi":
						switch (value) 
						{
							case "nao":
								synthesizer.SpeakAsync("Hello");
								robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_HELLO);
								break;
						}
						break;
					case "stand":
						switch (value)
						{
							case "up":
								synthesizer.SpeakAsync("Oops");
								robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_STAND_UP);
								break;
						}
						break;
				}
			}

			//EnableSpeechRecog(true);
		}


		private void Window_KeyDown(object sender, KeyEventArgs e)
		{
			// ... Test for F5 key.
			if (e.Key == Key.F5)
			{
				this.Title = "You pressed F5";
			}
		}

		private void OnButtonKeyDown(object sender, KeyEventArgs e)
		{

			switch (e.Key)
			{
				case Key.Up:
					robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_UP);
					break;
				case Key.Down:
					robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_DOWN);
					break;
				case Key.Left:
					robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_LEFT);
					break;
				case Key.Right:
					robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_RIGHT);
					break;

			}

	
			
		}


		private void BeginGame() {


			synthesizer.SpeakAsync("Okay Guess my color.");
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_COLOR_OFF);
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_SHOOT);
		}


		private void StartGame() 
		{
			colorGame = new ColorGame();
			SetScores();
		}

		private void SetScores() 
		{
			userScoreTextBox.Text = colorGame.participant.Score.ToString();
			robotScoreTextBox.Text = colorGame.robot.Score.ToString();
			winnerLabel.Content = "";
		}


		private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			speechRecognizer.Dispose();
		}

		private void TopButton_Click(object sender, RoutedEventArgs e)
		{
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_UP);
		}

		private void BackButton_Click(object sender, RoutedEventArgs e)
		{
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_DOWN);
		}

		private void RightButton_Click(object sender, RoutedEventArgs e)
		{
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_RIGHT);
		}

		private void LeftButton_Click(object sender, RoutedEventArgs e)
		{
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_KEYBOARD_LEFT);
		}

		private void Button_Click(object sender, RoutedEventArgs e)
		{
			StartGame();
			BeginGame();
		}

		private void StansUpButton_Click(object sender, RoutedEventArgs e)
		{
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_STAND_UP);
		}

		private void TurnLeft40Button_Click(object sender, RoutedEventArgs e)
		{
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_TURN_LEFT_40);
		}

		private void TurnRight40Button_Click(object sender, RoutedEventArgs e)
		{
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_TURN_RIGHT_40);
		}

		private void TurnAroundButton_Click(object sender, RoutedEventArgs e)
		{
			robotManager.SendCommandToNao((int)RobotManager.KeyboardCodes.WB_TURN_AROUND);
		}
	}


}
