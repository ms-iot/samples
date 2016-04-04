/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : XMLParser.cpp
 * Compiler          : Dev-C++ 4.9.8.0 - http://bloodshed.net/dev/
 * Revision          : $Revision: 3419 $
 * Date              : $Date: 2008-02-22 09:56:34 +0100 (fr, 22 feb 2008) $
 * Updated by        : $Author: khole $
 *
 * Support mail      : avr@atmel.com
 *
 * Target platform   : Win32
 *
 * AppNote           : AVR911 - AVR Open-source Programmer
 *
 * Description       : A simple XML DOM-like parser. It builds a complete tree from
 *                     the XML file. IT supports <section/> tags, but not tag attributes.
 *
 * 
 ****************************************************************************/
#include "XMLParser.hpp"

/* Private classes */

enum XMLNodeType
{
	xml_node,
	xml_subtree
};

/* Abstract class. XMLTree and XMLNode is derived from this class */
class XMLAbstractNode
{
	protected:
		string name; // Name of this node.
		XMLNodeType type;

	public:
		/* Constructor */
		XMLAbstractNode( const string & _name, XMLNodeType _type );

		/* Destructor */
		~XMLAbstractNode();

		/* Methods */
		const string & getName();
		XMLNodeType getType();

		bool isName( const string & _name ); // Compare name to _name.

		virtual void print() = 0;
};


/* Class describing a subtree, derived from XMLAbstractNode */
class XMLTree : public XMLAbstractNode
{
	protected:
		list<XMLAbstractNode *> nodes; // Nodes contained in this tree.

	public:	
		/* Constructor */
		XMLTree( const string & _name );

		/* Destructor */
		~XMLTree();

		/* Methods */
		void addNode( XMLAbstractNode * newnode );
		bool containsNode( const string & _name ); // Searches for name in list.
		XMLAbstractNode * getNode( const string & _name );

		void print();
};


/* Class describing an ordinary string-valued node, derived from XMLAbstractNode */
class XMLNode : public XMLAbstractNode
{
	protected:
		string value; // String value.

	public:
		/* Constructor */
		XMLNode( const string & _name, const string & _value );

		/* Destructor */
		~XMLNode();

		/* Methods */
		bool isEmpty(); // Contains an empty string?
		const string & getValue();

		void print();
};	




void XMLFile::removeComments( string & txt )
{
	long pos = 0; // Everything up to this point is clean.

	long startFoundAt; // Comment start and end found at these positions.
	long endFoundAt;

	/* Search and remove all comment tags */
	do
	{
		/* Search for comment start */
		startFoundAt = txt.find( "<!--", pos );

		/* Exit the search loop if no comment is found */
		if( startFoundAt == string::npos )
		{
			pos = txt.size();
			break;
		}

		/* Search for comment end */
		endFoundAt = txt.find( "-->", startFoundAt );

		/* Error if start but no end is found */
		if( endFoundAt == string::npos )
			throw new ErrorMsg( "Unclosed comment tag encountered! "
					"Comment start-tag '<!--' found, but no "
					"closing '-->'." );

		/* Remove comment tag */
		txt.erase( startFoundAt, endFoundAt - startFoundAt + 3 );

		pos = startFoundAt; // Prepare for next search.

	} while( pos < txt.size() );
}

void XMLFile::removeStartXML( string & txt )
{
	long pos = 0; // Everything up to this point is clean.

	long startFoundAt; // Comment start and end found at these positions.
	long endFoundAt;

	/* Search and remove the <?xml tag */
	startFoundAt = txt.find( "<?xml", pos );

	/* Exit the search loop if not found */
	if( startFoundAt == string::npos )
	{
		pos = txt.size();
		return;
	}

	/* Search for end */
	endFoundAt = txt.find( ">", startFoundAt );

	/* Remove tag */
	txt.erase( startFoundAt, endFoundAt - startFoundAt );
}


void XMLFile::removeAttributes( string & txt )
{
	long pos; // Everything up to this point is clean.

	long startFoundAt; // Tag start and end found at these positions.
	long endFoundAt;

	long spaceFoundAt; // Space before attribute found at this position.
	long slashFoundAt; // Ending slash found at this position.


	/* Convert all whitespace to plain spaces, just to make things easier */
	for( pos = 0; pos < txt.size(); pos++ )
	{
		if( txt[pos] == '\n' || txt[pos] == '\r' || txt[pos] == '\t' )
			txt.replace( pos, 1, " " );
	}

	pos = 0;

	/* Search and clean all tags */
	do
	{
		/* Search for tag start */
		startFoundAt = txt.find( "<", pos );

		/* Exit loop if no tag is found */
		if( startFoundAt == string::npos )
			break;

		/* Search for comment end */
		endFoundAt = txt.find( ">", startFoundAt );

		/* Error if start but no end is found */
		if( endFoundAt == string::npos )
			throw new ErrorMsg( "Unclosed tag encountered! "
					"Tag start token '<' found, but not "
					"closing '>'." );

		/* Remove whitespace before tag name */
		while( txt[startFoundAt+1] == ' ' )
		{
			txt.erase( startFoundAt+1, 1 ); // Remove.
			endFoundAt--; // String has now shrunk.
		}

		/* Search for space before attributes */
		spaceFoundAt = txt.find( " ", startFoundAt );
		if( spaceFoundAt < endFoundAt && spaceFoundAt != string::npos ) // Space found inside tag?
		{
			// If empty tag, we dont want to remove the / in />
			if ( txt.at(endFoundAt-1) == '/' )
			{
			 endFoundAt--;
			}
              
			/* Remove attributes */
			txt.erase( spaceFoundAt, endFoundAt - spaceFoundAt );
			endFoundAt -= endFoundAt - spaceFoundAt; // String has now shrunk.
		}


		pos = endFoundAt + 1; // Prepare for next search.

	} while( pos < txt.size() );
}


void XMLFile::readFile( const string & _filename )
{
	ifstream f; // XML file stream.
	string contents; // XML file contents.
	string templine;

	/* Attempt to open file */
	f.open( _filename.c_str(), ios::in );
	if( !f )
		throw new ErrorMsg( "Error opening XML file for input!" );

	/* Read everything into the contents string */
	contents.erase();
	templine.erase();
	do
	{
		contents += templine + " ";
		f >> templine; // This will cause EOF only when reading from the end.
	} while( !f.eof() );

	f.close();

	/* Remove comments and tag attributes */
	removeComments( contents );
	removeAttributes( contents );
	removeStartXML ( contents );

	/* Create root node */
	root = new XMLTree( "root" );
	parseFragment( contents, root );	

	Util.progress( "\r\n" ); // Finish progress indicator.
}


void XMLFile::parseFragment( string & fragment, XMLTree * parent )
{
	long startFoundAt; // Tag start and end found at these positions.
	long endFoundAt;

	string closingString; // Search string used for finding closing tag.
	long closingFoundAt; // Closing tag found at this position.

	string tagName; // These are for recently created nodes.
	string tagValue;

	XMLTree * newTree;
	XMLNode * newNode;	

	long nestedFoundAt; // Nested tags found at this position.

	/* Find top level tags */
	Util.progress( "#" ); // Advance progress indicator.

	while( true ) // Wait for break from inside.
	{
		/* Find start of tag */
		startFoundAt = fragment.find( "<", 0 );
		if( startFoundAt == string::npos ) // Exit loop if no tags found.
			break;

		/* Check if this is a closing tag for a higher level tag pair */
		if( fragment[startFoundAt+1] == '/' )
			break; // Exit loop.

		/* Find end of tag */
		endFoundAt = fragment.find( ">", startFoundAt );
		if( endFoundAt == string::npos ) // Error if end not found.
			throw new ErrorMsg( "Unclosed tag encountered! "
					"Tag start token '<' found, but no "
					"closing '>'." );

		/* Extract name of tag */
		tagName = fragment.substr( startFoundAt+1, endFoundAt-startFoundAt-1 );
		if( tagName.size() == 0 ) // Error if zero-length tag name.
			throw new ErrorMsg( "Unnamed tag encountered! "
					"No text between '<' and '>'." );

		/* Remove tag from fragment */
		fragment.erase( 0, startFoundAt+tagName.size()+2 );

		/* Check if it is an empty tag */
		if( tagName[tagName.size()-1] == '/' )
		{
			/* Create a new empty ordinary node */
			tagName.erase( tagName.size()-1 ); // Remove the slash.
			tagValue.erase(); // This tag has no value.
			newNode = new XMLNode( tagName, tagValue );
			parent->addNode( newNode );
		} else
		{
			/* Find the matching closing tag for this pair */
			closingString.erase();
			closingString += "</" + tagName + ">";
			closingFoundAt = fragment.find( closingString, 0 );
			if( closingFoundAt == string::npos ) // Error if not found.
				throw new ErrorMsg( "Closing tag not found! "
						"Opening tag '<" + tagName + ">' found, "
						"but not closing '" + closingString + "'." );

			/* Check for tags inside this tag pair, indicating a subtree */
			nestedFoundAt = fragment.find( "<", 0 );
			if( nestedFoundAt == closingFoundAt ) // No other tags inside?
			{
				/* Extract contents within tag pair */
				tagValue = fragment.substr( 0, closingFoundAt );

				/* Create new ordinary node */
				newNode = new XMLNode( tagName, tagValue );
				parent->addNode( newNode );
			} else
			{
				/* Create new subtree and parse it's fragment */
				newTree = new XMLTree( tagName );
				parent->addNode( newTree );
				parseFragment( fragment, newTree );

				/* Check that we can still find the closing tag */
				closingFoundAt = fragment.find( closingString, 0 );
				if( closingFoundAt == string::npos )
					throw new ErrorMsg( "Closing tag not found! "
							"Opening tag '<" + tagName + ">' found, "
							"but not closing '" + closingString + "'." );
			}

			/* Remove value and closing tag from fragment */
			fragment.erase( 0, closingFoundAt + closingString.size() ); 
		}
	};
}    			


/* Constructor */
XMLFile::XMLFile( const string & _filename )
{
	readFile( _filename );
}


/* Destructor */
XMLFile::~XMLFile()
{
	if( root != NULL )
		delete root;
}


bool XMLFile::exists( const string & path )
{
	XMLAbstractNode * currentNode = root;
	XMLTree * currentTree;
	long namePos; // Position for current tag name in path.
	long separatorPos; // Position for #-separator following tag name.
	string tagName; // Current tag name.

	namePos = 0;

	while( true ) // This will break out from the inside.
	{
		/* Find separator or set pos to end of text */
		separatorPos = path.find( "\\", namePos );
		if( separatorPos == string::npos )
			separatorPos = path.size();

		/* Extract tag name and check if it exists */
		tagName = path.substr( namePos, separatorPos-namePos );
		currentTree = (XMLTree *) currentNode; // It is indeed a tree.
		if( !currentTree->containsNode( tagName ) )
			return false; // Not found.

		currentNode = currentTree->getNode( tagName );

		/* Are there more tags in the path? */
		if( separatorPos < path.size() )
		{
			/* Now, the current node better be a tree */
			if( currentNode->getType() != xml_subtree )
			{
				return false; // Not found.
			} else
			{
				namePos = separatorPos + 1; // Advance position in path.
			}
		} else
		{
			break; // Found, exit loop.
		}
	}

	return true; // Found!
}


const string & XMLFile::getValue( const string & path )
{
	XMLAbstractNode * currentNode = root;
	XMLTree * currentTree;
	long namePos; // Position for current tag name in path.
	long separatorPos; // Position for #-separator following tag name.
	string tagName; // Current tag name.

	namePos = 0;

	while( true ) // This will break out from the inside.
	{
		/* Find separator or set pos to end of text */
		separatorPos = path.find( "\\", namePos );
		if( separatorPos == string::npos )
			separatorPos = path.size();

		/* Extract tag name and check if it exists */
		tagName = path.substr( namePos, separatorPos-namePos );
		currentTree = (XMLTree *) currentNode; // It is indeed a tree.
		if( !currentTree->containsNode( tagName ) )
			throw new ErrorMsg( "Node '" + tagName + "' not found!" );

		currentNode = currentTree->getNode( tagName );

		/* Are there more tags in the path? */
		if( separatorPos < path.size() )
		{
			/* Now, the current node better be a tree */
			if( currentNode->getType() != xml_subtree )
			{
				throw new ErrorMsg( "Illegal path: (" + path + ")!" );
			} else
			{
				namePos = separatorPos + 1; // Advance position in path.
			}
		} else
		{
			break; // Found, exit loop.
		}
	}

	/* Check that the current node is an ordinary node */
	if( currentNode->getType() != xml_node )
		throw new ErrorMsg( "Node '" + tagName + "' is not an element!" );

	return ((XMLNode *) currentNode)->getValue();
}


void XMLFile::print()
{
	root->print();
}


/* Constructor */
XMLAbstractNode::XMLAbstractNode( const string & _name, XMLNodeType _type ) :
	name( _name ),
	type( _type )
{
	// Node code here.
}


/* Destructor */
XMLAbstractNode::~XMLAbstractNode()
{
	// No code here.
}


const string & XMLAbstractNode::getName()
{
	return name;
}


XMLNodeType XMLAbstractNode::getType()
{
	return type;
}


bool XMLAbstractNode::isName( const string & _name )
{
	return (name == _name);
}


/* Constructor */
XMLTree::XMLTree( const string & _name ) :
	XMLAbstractNode::XMLAbstractNode( _name, xml_subtree )
{
	// No code here.
}


/* Destructor */
XMLTree::~XMLTree()
{
	/* Create an iterator for the list */
	list<XMLAbstractNode *>::iterator i;

	/* Destruct all contained nodes */
	for( i = nodes.begin(); i != nodes.end(); i++ )
	{
		delete (*i);
	}	
}	


void XMLTree::addNode( XMLAbstractNode * newnode )
{
	nodes.push_back( newnode );
}


bool XMLTree::containsNode( const string & _name )
{
	/* Create an iterator for the list */
	list<XMLAbstractNode *>::iterator i;

	/* Search for the node with name _name */
	i = nodes.begin();

	while( i != nodes.end() )
	{
		if( (*i)->isName( _name ) )
			return true;

		i++;
	}

	return false;
}


XMLAbstractNode * XMLTree::getNode( const string & _name )
{
	/* Create an iterator for the list */
	list<XMLAbstractNode *>::iterator i;

	/* Search for the node with name _name */
	i = nodes.begin();

	while( i != nodes.end() )
	{
		if( (*i)->isName( _name ) )
			return *i;

		i++;
	}

	return NULL;
}


void XMLTree::print()
{
	/* Create an iterator for the list */
	list<XMLAbstractNode *>::iterator i;

	cout << "TREE[ Name: \"" << name << "\" ]:" << endl;

	/* Search for the node with name _name */
	i = nodes.begin();

	while( i != nodes.end() )
	{
		(*i)->print();

		i++;
	}

	cout << ":END[\"" << name << "\"]" << endl;
}


/* Constructor */
XMLNode::XMLNode( const string & _name, const string & _value ) :
	XMLAbstractNode::XMLAbstractNode( _name, xml_node ),
	value( _value )
{
	// No code here.
}


/* Destructor */
XMLNode::~XMLNode()
{
	// Node code here.
}


bool XMLNode::isEmpty()
{
	return value.empty();
}


const string & XMLNode::getValue()
{
	return value;
}


void XMLNode::print()
{
	cout << "NODE[ Name: \"" << name << "\" Value: \"" << value << "\" ]" << endl; 
}

/* end of file */

