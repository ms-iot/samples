# Guidelines

All of these rules must be met:
	1. You must have a page on https://developer.microsoft-int.com/en-us/windows/iot/samples that corresponds to your sample. Visit the [ms-iot/content](https://github.com/ms-iot/content) on instructions.
		1. It must contain some explanation of what the sample is and how it works
		2. The same title must be used everywhere
		3. You must link back to your sample on ms-iot/samples in the header of your file. See [ms-iot sample template](https://github.com/ms-iot/content-private/blob/develop/Resources/contribute/template/sample-template.md)
	2. You must include a Readme with your sample
		1. Follow this [template](<LINK>)


## Guidelines

When creating a new sample, or updating an exisitng one, there are a handful of things to keep in mind.

1. **Creating a new sample:**

  When creating new content, please start with the following templates (view in *Raw* mode):


2. **Name your folder correctly**
  
  Please use upper camel case (e.g. BackgroundColor) when naming your files. 
  Use the same name everywhere
  Try to be as descriptive as possible without making your titles too long (good descriptions makes everyone's life easier) 

  
  * Docs: `content/en-us/docs/PowerShell.md` 
  * Samples: `content/en-us/samples/helloworld.md`


## Best Practices

### Do not check in binaries
Once a binary is added to the repository, it will be there forever.

Please do not add binaries to Git including:
* The output from a build (debug/release)
* SDF file (code database)
* Nuget package directories

Acceptable binaries:
* PNG, JPG, or other image formats

**Title and headings** 

* Title and headings are **all sentence-case** (only first word and proper nouns capitalized).


For more information, see the Windows Open Publishing Guide at http://aka.ms/windows-op-guide.



### References

1. [GitHub Documentation](https://help.github.com/)
2. [Git Cheatsheet!](https://github.com/github/training-materials/blob/master/downloads/github-git-cheat-sheet.pdf?raw=true)
3. [Git Documentation](http://www.git-scm.com/book/en/)

___

### How to contribute

1. [Get set up](get-setup.md)
2. [Making changes](making-changes.md) 
3. **[Authoring guidelines and best practices](authoring-guidelines.md)**

