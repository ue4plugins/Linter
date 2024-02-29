# Linter

Linter for Unreal Blueprints. Based of the [Linter v2](https://ue5.style), made compatible with Unreal Engine 5.
The original version can still be found on the [Marketplace](https://www.unrealengine.com/marketplace/product/linter-v2) or the original [GitHub Repository](https://github.com/ue4plugins/Linter).

## Usage
The Linter can be used in two ways:

### Usage in Editor
The Linter can be used to lint Assets in the Editor. 
To do so, select the Assets you want to lint. These can be single Assets or multiple Assets/Folders.
When right-clicking on a selected Asset, you can find the **Linter** option at the bottom the context menu:

### Usage as Commandlet
The Linter can also be used as a Commandlet and executed from the command line:
`UnrealEditor-Cmd.exe <ProjectPath> -run=Linter <Path(s)>`
If no Path is provided, the Linter will default to `/Game` and lint the whole Content Directory.

Advanced Options:
- `-RuleSet=<RuleSetName>` Specify a custom LintRuleSet to use (see below). Default: **Marketplace**
- `-json and -json=<Path>` Write the Linter output to a JSON file. Default: **\<ProjectPath\>/Saved/LintReports/**
- `-html and -html=<Path>` Write the Linter output to a interactive HTML file. Default: **\<ProjectPath\>/Saved/LintReports/**
- Both `-json` and `-html` can be absolute or relative paths and can contain a filename for the file. Both Options can be used at the same time.

### Understanding RuleSets
The Linter works with a Collection of LintRules, which are grouped into RuleSets.
Each LintRule can check for a specific quality or style issue in a Blueprint.
The Linter provides 2 RuleSets by default: 
- `Marketplace` checks against the [Epic Games Marketplace Guidelines](https://www.unrealengine.com/marketplace-guidelines)
- `ue4.style` checks against the [ue4.style](http://ue4.style/) guidelines.

You can define your own RuleSets by creating a new DataAsset of type `LintRuleSet` and adding LintRules to it.
To create new LintRules, subclass `ULintRule` and override the `PassesRule` function.
When using the Linter as a Commandlet, you can specify a RuleSet, predefined or your own, with the `-RuleSet` option.

## Installation

### Into a Project
To install the Linter into a Project, simply clone the `Linter` folder into the `Plugins` folder of your Project.

### Into the Engine
To install the Linter into the Engine, clone the `Linter` into some Place on your Computer.
You then need to Build the Plugin using the UAT and copy the built Plugin into the `Engine/Plugins/Marketplace/` folder.

```bash
# Clone Repository
git clone https://github.com/jwindgassen/Linter.git Linter

# Build Plugin. Both Paths must be absolute, otherwise UAT will complain.
# You cannot Build the Plugin directly into the Engine folder!
<path-to-engine>/Engine/Build/BatchFiles/RunUAT.bat BuildPlugin -Plugin=`pwd`/Linter/Linter.uplugin -Package=`pwd`/Linter_

# Copy Plugin into Engine and Clean up
cp -r Linter_ <path-to-engine>/Engine/Plugins/Marketplace/Linter
rm -r Linter Linter_
```

If you have [ue4cli](https://github.com/adamrehn/ue4cli) installed (which I highly recommend), you can simply use `ue4 uat` 
instead of `<path-to-engine>/Engine/Build/BatchFiles/RunUAT.bat`    
