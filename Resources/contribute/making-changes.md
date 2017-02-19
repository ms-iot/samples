# Making changes

Once your Git flow is set up, you can make changes to the repository.

For returning users - Ensure that you are up to date with the ms-iot repo:

  `git fetch ms-iot`

  `git reset --hard ms-iot/develop`

  `git push origin develop`
  
**Note**: This will delete all local changes and push ms-iot to your fork, so be sure this is what you want

1. Read the [rules](authoring-guidelines.md).
  1. Changes that do not follow the guidelines will be rejected
  2. Test your code!! 
  3. If you are adding a new document, ensure that the files and images are saved in the correct folders
2. Commit changes 
  1. When ready, add the files you want to to be staged for commit:
    * `git add [file]`
  2. Once done, commit your changes:
    * `git commit -m "[descriptive message]" `

### Submitting a Pull Request

A pull request (PR) is a request to the ms-iot team to accept your changes. It must first be approved by the team, but once accepted, it will immediately go live.

1. Grab the latest changes from upstream

    `git fetch ms-iot`
    
2. Merge it into your local repository
    1. `git merge ms-iot/develop`
    2. If you run into conflicts, you will have to hand-merge (you can use your favorite merging tool or even notepad for this)
    3. After hand-merging, you can continue the merge
        * `git add [fileYouHandMerged]`
        
3. Check your repositories status
  * `git status`. You may need to continue the merge to finish

4. Push your changes to your forked repository.
    * `git push origin develop`
    
5. Submit your pull request from your forked repository using the GitHub website

  ![Pull request](../images/contribute/newPR.png) 

6. If it's your first pull request, sign the Contribution License Agreement 
  1.	Without completing this step, we unfortunately cannot accept a pull request. You only need to do this once 

### Making changes to a pull request

1. Make your new changes, fetch upstream, merge upstream, and push your changes
    * If your pull request was never closed, you should not have to submit a new pull request. It should automatically update
    
### Git guidelines

When evaluating PRs, the following guidelines must be met:

1. Must be auto-mergable.
    * You should have done the work so that we can automatically merge your changes with the current state of the repository. Otherwise we will reject your pull-request and wait until you have fixed it
    * If, after you make your changes and submit your PR, you find it cannot be automerged, simply update your fork with the latest from our repo and fix the conflicts - that should make it auto-mergable
2. Never have duplicate commits

___

### How to contribute

1. [Get set up](get-setup.md)
2. **[Making changes](making-changes.md)** 
3. [Authoring guidelines and best practices](authoring-guidelines.md)