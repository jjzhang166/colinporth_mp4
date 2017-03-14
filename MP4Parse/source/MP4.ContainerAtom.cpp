#include "MP4.ContainerAtom.h"

using namespace MP4;

//{{{
ContainerAtom::ContainerAtom( char * type )
{
    this->_type = type;

    std::transform( this->_type.begin(), this->_type.end(), this->_type.begin(), ::toupper );
}

//}}}
//{{{
ContainerAtom::~ContainerAtom( void )
{
    std::multimap< std::string, Atom * >::iterator it;

    for( it = this->_children.begin(); it != this->_children.end(); ++it )
    {
        delete ( Atom * )( it->second );
    }

}
//}}}

//{{{
void ContainerAtom::addChild( Atom * a )
{
    if( a == NULL )
    {
        return;
    }

    this->_children.insert( std::pair< std::string, Atom * >( a->getType(), a ) );
}
//}}}
//{{{
bool ContainerAtom::hasChildren( void )
{
    return _children.size() > 0;
}
//}}}
//{{{
unsigned int ContainerAtom::numberOfChildren( void )
{
    return _children.size();
}
//}}}

//{{{
std::string ContainerAtom::description( void )
{
    std::string s;
    std::multimap< std::string, Atom * >::iterator it;

    s += "MP4 Container Atom: " + this->_type + "\n";

    for( it = this->_children.begin(); it != this->_children.end(); ++it )
    {
        s.append( ( ( Atom * )( it->second ) )->description() );
    }

    return s;
}
//}}}

//{{{
Atom* ContainerAtom::findChild( const std::string &type )
{
    if ( this->_type == type )
    {
      return this;
    }

    std::multimap< std::string, Atom * >::iterator it;
    for ( it = this->_children.begin(); it != this->_children.end(); ++it )
    {
        ContainerAtom *containerAtom = dynamic_cast< ContainerAtom* >( it->second );
        if ( containerAtom )
        {
            return containerAtom->findChild( type );
        }
        else
        {
            Atom *atom = ( Atom* )it->second;
            if ( atom->getType() == type )
            {
                return atom;
            }
        }
    }

    return NULL;
}
//}}}
