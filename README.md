# Command Menu Generator
A generator made for custom voice menus for TF2, but may work for other games made in the Source Engine. Refer to the Introduction wiki for more information.

If you are on a non-WinNT OS then use the [Wine](https://www.winehq.org/) program to run the generator. If [Wine](https://www.winehq.org/) is not supported (like MacOS Catalina), then use a Virtual Machine with Windows (or the Bootcamp software that comes with MacOS). Alternatively you can just compile from the source code as this program does not use any Operating System specific code.

# What is this?

A voice command is a pre-defined call that makes the player say a voice line. They are organized in voice menus, which are lists of voice commands.

Voice menus can be replicated with binds to run *any* command rather than just voice commands. These are called command menus. Command menus can also recursively run other command menus. Examples of this behavior are in Concise Menu scripts.<sup>[\[1\]](https://old.reddit.com/r/Tf2Scripts/comments/42php2/i_made_a_concise_taunt_menu_that_works_similarly)[\[2\]](https://old.reddit.com/r/tf2scripthelp/wiki/complexscripts#wiki_concise_voice_menu)</sup> It's hard to see what state command menus are in however, so you can also hook up binds to show them in console or [captions on your HUD](https://old.reddit.com/r/tf2scripthelp/wiki/captions).

This program aims to automate the process of creating command menus, as they can take a long time to make if you have a lot of binds (like ≥20 binds) to make and want a display as well, so that you can focus most of your time on desiging a good command menu.

# Performance in Source Games

Game | Functionality | Caption Display | Console Display
-----|---------------|----------|-------------
Team Fortress 2 | Flawless | Flawless | Flawless
Counter-Strike: Global Offensive | Ok<sup>[1]</sup> | Broken | Good<sup>[2]</sup>

<sup>[1]</sup> - Compiled command menus must be written directly to the csgo/cfg folder to function. (Do this by compiling to the csgo folder.)

<sup>[2]</sup> - Command menus will not display if a command menu was not already ran.

Any source games that I left out are untested. If you encounter issues while using this program, then make an issue on what issues you encountered and if it is possible to fix.