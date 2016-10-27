---
layout: sample
title: <title>
description: <1 to 2 sentence description about this page - used for SEO>
keyword: <list any keywords not in the title that you want to be used for search>
permalink: <URL needs to match filename, and should be descriptive.  Should also include locale and parent folders.  For example, AllJoyn.md - /en-US/Docs/AllJoyn.htm, Downloads - /en-US/Downloads.htm>
samplelink: https://github.com/ms-iot/samples/
lang: en-US
---
?
Edit the above information for you topic!
?
# Title of the page, that matches title in metadata above.  Only one H1 can be used.
?
## The rest of the headings
___
?
### can be used at your discretion
?
#### but do try to keep a logical heirarchy
1. You can also use numbering in your document.
2. This is an example of a two lines of numbered list
?
Below is an image
?
![This is the alt text for the image]({{site.baseurl}}/Resources/images/Octocat.png)
?
For white images, add a border - be sure to not include the `site.baseurl` when using this:
?
{% include imageborder.html alt="This is the alt text for the image" link="/Resources/images/MSConnectSignup.png" %}
?
There are different formatting options for extra emphasis
?
{% include note.html text="This is a note" %}
?
{% include warning.html text="This is a warning" %}
?
{% include tip.html text="This is a tip" %}
?
{% highlight CS %}
This is a code snippet
?
It can be multiple lines
?
Also notice the specification of language above - see the following link for more information on what's available
?
https://jekyllrb.com/docs/templates/#code-snippet-highlighting
{% endhighlight %}
?
Also, you can have `inline code snippets`
?
| Here | Is | A  | Table |
|------|----|----|-------|
| Here      | Are  | Some  | Table  |
| Contents  | 1    | 2     | 3      |
| 4         | 5    | 6     | 7      |   
