var ie = document.all != null;
var moz = !ie && document.getElementById != null && document.layers == null;
function emulateHTMLModel()
{
// copied from http://www.webfx.nu/dhtml/ieemu/htmlmodel.html

// This function is used to generate a html string for the text properties/methods
// It replaces '\n' with "<BR"> as well as fixes consecutive white spaces
// It also repalaces some special characters	
function convertTextToHTML(s) {
    s = s.replace(/\&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/\n/g, "<BR>").replace(/\t/g, "    "); //tachyon
    while (/\s\s/.test(s))
        s = s.replace(/\s\s/, "&nbsp; ");
    return s.replace(/\s/g, " ");
}


HTMLElement.prototype.__defineSetter__("innerText", function (sText) {
    this.innerHTML = convertTextToHTML(sText);
    return sText;		
});

var tmpGet;
HTMLElement.prototype.__defineGetter__("innerText", tmpGet = function () {
    var r = this.ownerDocument.createRange();
    r.selectNodeContents(this);
    return r.toString();
});

}

if (moz) 
emulateHTMLModel();


// Regular Expressions largely copied from Cory Johns (darkness@yossman.net) excellent Syntax::Highlight::Perl module (see http://search.cpan.org/~johnsca/)

var re;
var RE  = new Array;

// quoted string
re  = /('|"|`).*?\1/;
RE[0]  = new RegExp(re);

// comment
re  = /\#.*?([\r\n]+|$)/; //tachyon
RE[1]  = new RegExp(re);
 
// operator
re = /xor|\.\.\.|and|not|\|\|\=|cmp|\>\>\=|\<\<\=|\<\=\>|\&\&\=|or|\=\>|\!\~|\^\=|\&\=|\|\=|\.\=|x\=|\%\=|\/\=|\*\=|\-\=|\+\=|\=\~|\*\*|\-\-|\.\.|\|\||\&\&|\+\+|\-\>|ne|eq|\!\=|\=\=|ge|le|gt|lt|\>\=|\<\=|\>\>|\<\<|\,|\=|\:|\?|\^|\||x|\%|\/|\*|\<|\&|\\|\~|\!|\>|\.|\-|\+ /;
RE[2]  = new RegExp(re);

// builtin variables
re = /\$\#?_|\$(?:\^[LAECDFHIMOPRSTWX]|[0-9&`'+*.\/|,\\";#%=\-~^:?!@\$<>()\[\]])|\$\#?ARGV(?:\s*\[)?|\$\#?INC\s*\[|\$(?:ENV|SIG|INC)\s*\{|\@(?:_|ARGV|INC)|\%(?:INC|ENV|SIG)/;
RE[3] = new RegExp(re);

// variable class specifiers
re = /(?:(?:[\@\%\*]|\$\#?)\$*)/;
RE[4] = new RegExp(re);

// keyword
re  = /(continue|foreach|require|package|scalar|format|unless|local|until|while|elsif|next|last|goto|else|redo|sub|for|use|no|if|my)\b/;
RE[5]  = new RegExp(re);

// builtin function
re  = /(getprotobynumber|getprotobyname|getservbyname|gethostbyaddr|gethostbyname|getservbyport|getnetbyaddr|getnetbyname|getsockname|getpeername|setpriority|getprotoent|setprotoent|getpriority|endprotoent|getservent|setservent|endservent|sethostent|socketpair|getsockopt|gethostent|endhostent|setsockopt|setnetent|quotemeta|localtime|prototype|getnetent|endnetent|rewinddir|wantarray|getpwuid|closedir|getlogin|readlink|endgrent|getgrgid|getgrnam|shmwrite|shutdown|readline|endpwent|setgrent|readpipe|formline|truncate|dbmclose|syswrite|setpwent|getpwnam|getgrent|getpwent|ucfirst|sysread|setpgrp|shmread|sysseek|sysopen|telldir|defined|opendir|connect|lcfirst|getppid|binmode|syscall|sprintf|getpgrp|readdir|seekdir|waitpid|reverse|unshift|symlink|dbmopen|semget|msgrcv|rename|listen|chroot|msgsnd|shmctl|accept|unpack|exists|fileno|shmget|system|unlink|printf|gmtime|msgctl|semctl|values|rindex|substr|splice|length|msgget|select|socket|return|caller|delete|alarm|ioctl|index|undef|lstat|times|srand|chown|fcntl|close|write|umask|rmdir|study|sleep|chomp|untie|print|utime|mkdir|atan2|split|crypt|flock|chmod|BEGIN|bless|chdir|semop|shift|reset|link|stat|chop|grep|fork|dump|join|open|tell|pipe|exit|glob|warn|each|bind|sort|pack|eval|push|keys|getc|kill|seek|sqrt|send|wait|rand|tied|read|time|exec|recv|eof|chr|int|ord|exp|pos|pop|sin|log|abs|oct|hex|tie|cos|vec|END|ref|map|die|\-C|\-b|\-S|\-u|\-t|\-p|\-l|\-d|\-f|\-g|\-s|\-z|uc|\-k|\-e|\-O|\-T|\-B|\-M|do|\-A|\-X|\-W|\-c|\-R|\-o|\-x|lc|\-w|\-r)\b/;
RE[6]  = new RegExp(re);

// identifier (variable, subroutine, packages)
re = /(?:(?:[A-Za-z_]|::)(?:\w|::)*)/;
RE[7] = new RegExp(re);

// number
re = /0x[\da-fA-F]+|[_.\d]+([eE][-+]?\d+)?/;
RE[8] = new RegExp(re);


var classes  = new Array("quotedString", "comment", "operator", "builtinVariable", "variableSpecifier", "keyword", "builtinFunction", "identifier", "number");


/* This is the actual highlighting function.
 * Takes an html object as argument
 * returns nothing
 * replaces the text inside the html object with colored text using <span>'s
 * css is defined separately. See the array classes to find out the css class names.
 */
function HighlightCode(object)
{
	codeText = object.innerText; //HTML.replace(/<.*?>/g, "");
	object.innerHTML = '';
	var left;
	var match;
 	var right;
	while (codeText.length > 0)
	{
		var mode = -1 ;
		var index = 999999999;
		for (var i = 0; i < RE.length; i++)
		{
			if ((codeText.match(RE[i])) && (RegExp.leftContext.length < index))
			{
				left  = RegExp.leftContext;
				match = RegExp.lastMatch;
				right = RegExp.rightContext;
				index = RegExp.leftContext.length;
 				mode  = i;
			}
		}
		if (mode == -1)
		{
			object.appendChild(document.createTextNode(codeText)); //.replace(/\r\n/g, "\r")));
			codeText = '';
		}
		else
		{
			// append the plain text to the <code> block
			object.appendChild(document.createTextNode(left)); //.replace(/\r\n/g, "\r")));

			// create a new <span> with the current code
			var span = document.createElement("span");
			span.setAttribute("className", classes[mode]); // ie
			span.setAttribute("class", classes[mode]); //mozilla
			span.appendChild(document.createTextNode(match));
			object.appendChild(span);	

			codeText  = right;
		}
	}
}

// little bit of JQuery to highlight code in all pre elements
$(document).ready(function(){
    $("pre").each(function(i){
        HighlightCode(this);
     });
});

