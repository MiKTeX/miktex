/*
	wsdd.h WS-Discovery 1.1 2009 with WS-Addressing 2005/08

	Generated with:
	wsdl2h -cyex -o wsdd.h -t WS/WS-typemap.dat WS/discovery.xsd

        Modified by Robert van Engelen:

        - Removed //gsoapopt
        - Changed //gsoap wssd schema namespace directive to import directive
        - Added #import "wsdx.h" at the end of these definitions
	- Added #define SOAP_WSDD_2009
*/

#define SOAP_WSDD_2009

/******************************************************************************\
 *                                                                            *
 * Definitions                                                                *
 *   http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01                    *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Import                                                                     *
 *                                                                            *
\******************************************************************************/

#import "wsa5.h"	// wsa5 = <http://www.w3.org/2005/08/addressing>

/******************************************************************************\
 *                                                                            *
 * Schema Namespaces                                                          *
 *                                                                            *
\******************************************************************************/

#define SOAP_NAMESPACE_OF_wsdd	"http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01"
//gsoap wsdd  schema import:	http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01
//gsoap wsdd  schema elementForm:	qualified
//gsoap wsdd  schema attributeForm:	unqualified

/******************************************************************************\
 *                                                                            *
 * Built-in Schema Types and Top-Level Elements and Attributes                *
 *                                                                            *
\******************************************************************************/


// Imported element "http://www.w3.org/2005/08/addressing":EndpointReference declared as _wsa5__EndpointReference


/// Typedef synonym for struct wsdd__HelloType.
typedef struct wsdd__HelloType wsdd__HelloType;

/// Typedef synonym for struct wsdd__ByeType.
typedef struct wsdd__ByeType wsdd__ByeType;

/// Typedef synonym for struct wsdd__ProbeType.
typedef struct wsdd__ProbeType wsdd__ProbeType;

/// Typedef synonym for struct wsdd__ProbeMatchesType.
typedef struct wsdd__ProbeMatchesType wsdd__ProbeMatchesType;

/// Typedef synonym for struct wsdd__ProbeMatchType.
typedef struct wsdd__ProbeMatchType wsdd__ProbeMatchType;

/// Typedef synonym for struct wsdd__ResolveType.
typedef struct wsdd__ResolveType wsdd__ResolveType;

/// Typedef synonym for struct wsdd__ResolveMatchesType.
typedef struct wsdd__ResolveMatchesType wsdd__ResolveMatchesType;

/// Typedef synonym for struct wsdd__ResolveMatchType.
typedef struct wsdd__ResolveMatchType wsdd__ResolveMatchType;

/// Typedef synonym for struct wsdd__ScopesType.
typedef struct wsdd__ScopesType wsdd__ScopesType;

/// Typedef synonym for struct wsdd__SecurityType.
typedef struct wsdd__SecurityType wsdd__SecurityType;

/// Typedef synonym for struct wsdd__SigType.
typedef struct wsdd__SigType wsdd__SigType;

/// Typedef synonym for struct wsdd__AppSequenceType.
typedef struct wsdd__AppSequenceType wsdd__AppSequenceType;

/******************************************************************************\
 *                                                                            *
 * Schema Types and Top-Level Elements and Attributes                         *
 *   http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01                    *
 *                                                                            *
\******************************************************************************/


/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":QNameListType is a simpleType containing a whitespace separated list of xs:QName.
typedef _QName wsdd__QNameListType;

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":UriListType is a simpleType containing a whitespace separated list of xs:anyURI.
typedef char* wsdd__UriListType;

/// union of values "tns:FaultCodeType xs:QName"
typedef char* wsdd__FaultCodeOpenType;

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":FaultCodeType is a simpleType restriction of xs:QName.
enum wsdd__FaultCodeType
{
	wsdd__MatchingRuleNotSupported,	///< xs:QName value=""http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":MatchingRuleNotSupported"
};
/// Typedef synonym for enum wsdd__FaultCodeType.
typedef enum wsdd__FaultCodeType wsdd__FaultCodeType;

/******************************************************************************\
 *                                                                            *
 * Schema Complex Types and Top-Level Elements                                *
 *   http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01                    *
 *                                                                            *
\******************************************************************************/


/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":HelloType is a complexType.
struct wsdd__HelloType
{
/// Imported element reference "http://www.w3.org/2005/08/addressing":EndpointReference.
    _wsa5__EndpointReference             wsa5__EndpointReference        1;	///< Required element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Scopes.
    struct wsdd__ScopesType*             Scopes                         0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":XAddrs.
    wsdd__UriListType                    XAddrs                         0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":MetadataVersion.
    unsigned int                         MetadataVersion                1;	///< Required element.
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ByeType is a complexType.
struct wsdd__ByeType
{
/// Imported element reference "http://www.w3.org/2005/08/addressing":EndpointReference.
    _wsa5__EndpointReference             wsa5__EndpointReference        1;	///< Required element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Scopes.
    struct wsdd__ScopesType*             Scopes                         0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":XAddrs.
    wsdd__UriListType                    XAddrs                         0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":MetadataVersion.
    unsigned int*                        MetadataVersion                0;	///< Optional element.
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ProbeType is a complexType.
struct wsdd__ProbeType
{
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Scopes.
    struct wsdd__ScopesType*             Scopes                         0;	///< Optional element.
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ProbeMatchesType is a complexType.
struct wsdd__ProbeMatchesType
{
/// Size of array of struct wsdd__ProbeMatchType* is 0..unbounded
   $int                                  __sizeProbeMatch               0;
/// Array struct wsdd__ProbeMatchType* of length 0..unbounded
    struct wsdd__ProbeMatchType*         ProbeMatch                     0;
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ProbeMatchType is a complexType.
struct wsdd__ProbeMatchType
{
/// Imported element reference "http://www.w3.org/2005/08/addressing":EndpointReference.
    _wsa5__EndpointReference             wsa5__EndpointReference        1;	///< Required element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Scopes.
    struct wsdd__ScopesType*             Scopes                         0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":XAddrs.
    wsdd__UriListType                    XAddrs                         0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":MetadataVersion.
    unsigned int                         MetadataVersion                1;	///< Required element.
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ResolveType is a complexType.
struct wsdd__ResolveType
{
/// Imported element reference "http://www.w3.org/2005/08/addressing":EndpointReference.
    _wsa5__EndpointReference             wsa5__EndpointReference        1;	///< Required element.
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ResolveMatchesType is a complexType.
struct wsdd__ResolveMatchesType
{
/// Element ResolveMatch of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ResolveMatchType.
    struct wsdd__ResolveMatchType*       ResolveMatch                   0;	///< Optional element.
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ResolveMatchType is a complexType.
struct wsdd__ResolveMatchType
{
/// Imported element reference "http://www.w3.org/2005/08/addressing":EndpointReference.
    _wsa5__EndpointReference             wsa5__EndpointReference        1;	///< Required element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Scopes.
    struct wsdd__ScopesType*             Scopes                         0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":XAddrs.
    wsdd__UriListType                    XAddrs                         0;	///< Optional element.
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":MetadataVersion.
    unsigned int                         MetadataVersion                1;	///< Required element.
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":SecurityType is a complexType.
struct wsdd__SecurityType
{
/// Element reference "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Sig.
    struct wsdd__SigType*                Sig                            0;	///< Optional element.
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":SigType is a complexType.
struct wsdd__SigType
{
/// TODO: <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// Attribute Scheme of type xs:anyURI.
   @char*                                Scheme                         1;	///< Required attribute.
/// Attribute KeyId of type xs:base64Binary.
   @char*                                KeyId                          0;	///< Optional attribute.
/// Attribute Refs of type xs:IDREFS.
   @char*                                Refs                           1;	///< Required attribute.
/// Attribute Sig of type xs:base64Binary.
   @char*                                Sig                            1;	///< Required attribute.
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ScopesType is a complexType with simpleContent.
struct wsdd__ScopesType
{
/// __item wraps '"http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":UriListType' simpleContent.
    wsdd__UriListType                    __item                        ;
/// Attribute MatchBy of type xs:anyURI.
   @char*                                MatchBy                        0;	///< Optional attribute.
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":AppSequenceType is a complexType with complexContent restriction of xs:anyType.
struct wsdd__AppSequenceType
{
/// Attribute InstanceId of type xs:unsignedInt.
   @unsigned int                         InstanceId                     1;	///< Required attribute.
/// Attribute SequenceId of type xs:anyURI.
   @char*                                SequenceId                     0;	///< Optional attribute.
/// Attribute MessageNumber of type xs:unsignedInt.
   @unsigned int                         MessageNumber                  1;	///< Required attribute.
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/******************************************************************************\
 *                                                                            *
 * Additional Top-Level Elements                                              *
 *   http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01                    *
 *                                                                            *
\******************************************************************************/


/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Hello of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":HelloType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Bye of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ByeType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Probe of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ProbeType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ProbeMatches of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ProbeMatchesType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Resolve of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ResolveType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ResolveMatches of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ResolveMatchesType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Types of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":QNameListType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Scopes of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":ScopesType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":XAddrs of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":UriListType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":MetadataVersion of type xs:unsignedInt.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":SupportedMatchingRules of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":UriListType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Security of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":SecurityType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Sig of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":SigType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":AppSequence of type "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":AppSequenceType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/******************************************************************************\
 *                                                                            *
 * Additional Top-Level Attributes                                            *
 *   http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01                    *
 *                                                                            *
\******************************************************************************/


/// Top-level attribute "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01":Id of simpleType xs:ID.
/// Note: use wsdl2h option -g to auto-generate a top-level attribute declaration and processing code.

/******************************************************************************\
 *                                                                            *
 * XML Data Binding                                                           *
 *                                                                            *
\******************************************************************************/


/**

@page page_XMLDataBinding XML Data Binding

SOAP/XML services use data bindings contractually bound by WSDL and auto-
generated by wsdl2h and soapcpp2 (see Service Bindings). Plain data bindings
are adopted from XML schemas as part of the WSDL types section or when running
wsdl2h on a set of schemas to produce non-SOAP-based XML data bindings.

The following readers and writers are C/C++ data type (de)serializers auto-
generated by wsdl2h and soapcpp2. Run soapcpp2 on this file to generate the
(de)serialization code, which is stored in soapC.c[pp]. Include "soapH.h" in
your code to import these data type and function declarations. Only use the
soapcpp2-generated files in your project build. Do not include the wsdl2h-
generated .h file in your code.

XML content can be retrieved from:
  - a file descriptor, using soap->recvfd = fd
  - a socket, using soap->socket = ...
  - a C++ stream, using soap->is = ...
  - a buffer, using the soap->frecv() callback

XML content can be stored to:
  - a file descriptor, using soap->sendfd = fd
  - a socket, using soap->socket = ...
  - a C++ stream, using soap->os = ...
  - a buffer, using the soap->fsend() callback


@section wsdd Top-level root elements of schema "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01"

  - <wsdd:Hello> (use wsdl2h option -g to auto-generate)

  - <wsdd:Bye> (use wsdl2h option -g to auto-generate)

  - <wsdd:Probe> (use wsdl2h option -g to auto-generate)

  - <wsdd:ProbeMatches> (use wsdl2h option -g to auto-generate)

  - <wsdd:Resolve> (use wsdl2h option -g to auto-generate)

  - <wsdd:ResolveMatches> (use wsdl2h option -g to auto-generate)

  - <wsdd:Types> (use wsdl2h option -g to auto-generate)

  - <wsdd:Scopes> (use wsdl2h option -g to auto-generate)

  - <wsdd:XAddrs> (use wsdl2h option -g to auto-generate)

  - <wsdd:MetadataVersion> (use wsdl2h option -g to auto-generate)

  - <wsdd:SupportedMatchingRules> (use wsdl2h option -g to auto-generate)

  - <wsdd:Security> (use wsdl2h option -g to auto-generate)

  - <wsdd:Sig> (use wsdl2h option -g to auto-generate)

  - <wsdd:AppSequence> (use wsdl2h option -g to auto-generate)

*/

#import "wsdx.h"

/* End of wsdd2.h */
