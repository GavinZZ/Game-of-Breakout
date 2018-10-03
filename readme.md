Assignment One Readme

Welcome to the Game of Breakout. This game is designed and implemented by Yuanhao Zhang, with student ID 20658228.
In the splash screen, there are a few different modes, including normal mode, hard mode, impossible mode, and test(TA) mode.
The following are a list of feature/setup to my game.

1. Normal, hard, impossible mode have pre-determined FPS, speed and paddle length. Using any of these three modes will overwrite the specified FPS/speed given in the command-line parameters.
2. Test(TA) mode is designed for TA's to test the game. This mode is developed specifically for TA's as it mentioned in the assignment detail that "Assume that the TA marking your game is a terrible gamer". This mode will extend the width of the paddle to be the same with as the screen, so that TA's do not need to worry about losing the game while testing. Moreover, for Test(TA) mode, if there are command-line parameters specified, it will use the specified FPS/speed. Otherwise, the speed is set to fast(12) for the purpose of testing.
3. This game uses 'a' and 'd' to move the paddle. Once the ball reaches the bottom of the window, the game is over. However, users can move around the paddle to bounce back the balls.
4. I have added custom UI design, such as rainbow colour bricks, random paddle colours, etc. Furthermore, the brick will change colours depending on the numbers and level of bricks cleared.
5. I have set the window size to be 1280 * 700. The reason that the height is 700 is that the max height on my mac is 707. I have talked to Professor Jeff Avery about this; he said that if 1280 * 800 doesn't fit, we can use smaller window size.
6. Once all the bricks are cleared, the game will generate a new set of blocks and increase the speed if it hasn't reached max speed cap; if it has reacheed the speed cap, then the width of the block will decrease. These will apply to any modes excluding Test(TA) mode.
7. There are other functionality such as "God Mode" designed for normal/hard/impossible mode by pressing 'g', such that user will never lose the game. During anytime, you can press 'r' to return to the main menu, or 'q' to quit the game. There is also a hidden functionality called "Clear this Level" by pressing 'c', this will automatically finish the current level, re-generate all the bricks, and update the speed or paddle width.

Hope you have a enjoyable game experience at this Game of Breakout developed by Yuanhao Zhang!