# Command Menu Generator
A custom language made for efficiently generating custom recursive bind-voicemenus for TF2, but may work for other games made in the Source Engine. Refer to the Introduction wiki for more information.

If you are on a non-WinNT OS then use [Wine](https://www.winehq.org/) to run the generator, a Virtual Machine with Windows or the Bootcamp software that comes with MacOS (if Wine doesn't work), or just compile from the source code as this program does not use any OS specific code.

NOTE: Non Windows users can get the needed captioncompiler.exe for a Source Game by forcing installation of the Windows version of the specified Source Game (through Properties > Compatibility).

## "What is this?"

A voice command is a pre-defined call that makes the player say a voice line. They are organized in voice menus, which are lists of voice commands.

Voice menus can be replicated with binds to run *any* command rather than just voice commands. These types of voicemenus will be called command menus (shortened to cmd menu sometimes) for the sake of this program's documentation. Command menus can also recursively run other command menus. Examples of this behavior are in Concise Menu scripts.<sup>[\[1\]](https://old.reddit.com/r/Tf2Scripts/comments/42php2/i_made_a_concise_taunt_menu_that_works_similarly)[\[2\]](https://old.reddit.com/r/tf2scripthelp/wiki/complexscripts#wiki_concise_voice_menu)</sup> It's hard to see what state command menus are in however, so you can also hook up binds to show them in console or [captions on your HUD](https://old.reddit.com/r/tf2scripthelp/wiki/captions).

This program aims to automate the process of creating command menus, as they can take a long time to make if you have a lot of binds (like ≥20 binds) to make and want a display as well, so that you can focus most of your time on desiging a good command menu.

## "How do I use it?"

1. Download the generator through the "releases" tab. (You may have to scroll down to see it.)

2. Create a file with the contents being your choice. [You can use an example from this wiki.](https://github.com/WhyIsEvery4thYearAlwaysBad/CmdMenuGenerator/wiki/Examples) (If you're the section of text withCMenu code.)

3. Drag the file you created onto the generator. The folder "customvoicemenu" should appear. 
	* You can also just run the generator from command line (like cmd.exe or any BASH terminal) if you want more customization.

4. Drag the folder "customvoicemenu" into the folder path `<base source game directory>/<mod directory>/custom` (e.g `Team Fortress 2/tf/custom`). 
	
	* If there is no custom folder, then go into "customvoicemenu" folder and move the cfg folder in it into `<base source game directory>/<mod directory>/cfg` (e.g `Team Fortress 2/tf/cfg`) and move the `customvoicemenu/resource/cap` into `<base source game directory>/<mod directory>/resource`.
	
	* If there is no "resource" folder, then prepend "consolemode=\"true\"" in the CMenu code.

5. Now create a CFG file in `<base source game directory>/<mod directory>/cfg` or `<base source game directory>/<mod directory>/custom/<name of choice>/cfg` and choose the file contents of your choosing.

	* If you're using an [example](https://github.com/WhyIsEvery4thYearAlwaysBad/CmdMenuGenerator/wiki/Examples) then use the section of text with "CFG script code" above it.
	
6. Run the CFG script code and press the key you replaced "\<KEY\>" with. You should see some amount of text pop up. If you do see that, then you're done.

[More information in the wiki.](https://github.com/WhyIsEvery4thYearAlwaysBad/CmdMenuGenerator/wiki/Compiletime-and-Runtime)

## Performance in Source Games

Game | Functionality | Caption Display | Console Display
-----|---------------|----------|-------------
Team Fortress 2 | Flawless | Flawless | Flawless
Counter-Strike: Global Offensive | Ok<sup>[1]</sup> | Broken | Good<sup>[2]</sup>

<sup>[1]</sup> - Compiled command menus must be written directly to the csgo/cfg folder to function. (Do this by compiling to the csgo folder.)

<sup>[2]</sup> - Command menus will not display if a command menu was not already ran.

Any source games that I left out are untested. If you encounter issues while using this program, then make an issue on what issues you encountered and if it is possible to fix.

# Disclaimer

Any responsibilities that comes with using this generator (such as not spamming chat binds) are soley bound to the user. The creator (WhyIsEvery4thYearAlwaysBad or Amicdict) does not bear any responsibility for bans/mutes from malicious use of this program caused by other users of this program.
