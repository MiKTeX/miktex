/*
	wsdd10.h WS-Discovery 1.0 2005 with WS-Addressing 2004/08

	Generated with:
	wsdl2h -cyex -o wsdd10.h -t WS/WS-typemap.dat WS/discovery.xsd

        Modified by Robert van Engelen:

        - Removed //gsoapopt
        - Changed //gsoap wssd schema namespace directive to import directive
	- Changed to WS-Addressing 2004/08 protocol
	- Changed namespace to WS-Discovery 2005/04 protocol with wsdd10
        - Added #import "wsdx.h" at the end of these definitions
	- Added #define SOAP_WSDD_2005
*/

#define SOAP_WSDD_2005

/******************************************************************************\
 *                                                                            *
 * Definitions                                                                *
 *   http://schemas.xmlsoap.org/ws/2005/04/discovery
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Import                                                                     *
 *                                                                            *
\******************************************************************************/

#import "wsa.h"	// wsa = <http://schemas.xmlsoap.org/ws/2004/08/addressing>

/******************************************************************************\
 *                                                                            *
 * Schema Namespaces                                                          *
 *                                                                            *
\******************************************************************************/

#define SOAP_NAMESPACE_OF_wsdd	"http://schemas.xmlsoap.org/ws/2005/04/discovery"
//gsoap wsdd  schema import:	http://schemas.xmlsoap.org/ws/2005/04/discovery
//gsoap wsdd  schema elementForm:	qualified
//gsoap wsdd  schema attributeForm:	unqualified

/******************************************************************************\
 *                                                                            *
 * Built-in Schema Types and Top-Level Elements and Attributes                *
 *                                                                            *
\******************************************************************************/


// Imported element "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReference declared as _wsa__EndpointReference


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
 *   http://schemas.xmlsoap.org/ws/2005/04/discovery                          *
 *                                                                            *
\******************************************************************************/


/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":QNameListType is a simpleType containing a whitespace separated list of xs:QName.
typedef _QName wsdd__QNameListType;

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":UriListType is a simpleType containing a whitespace separated list of xs:anyURI.
typedef char* wsdd__UriListType;

/// union of values "tns:FaultCodeType xs:QName"
typedef char* wsdd__FaultCodeOpenType;

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":FaultCodeType is a simpleType restriction of xs:QName.
enum wsdd__FaultCodeType
{
	wsdd__MatchingRuleNotSupported,	///< xs:QName value=""http://schemas.xmlsoap.org/ws/2005/04/discovery":MatchingRuleNotSupported"
};
/// Typedef synonym for enum wsdd__FaultCodeType.
typedef enum wsdd__FaultCodeType wsdd__FaultCodeType;

/******************************************************************************\
 *                                                                            *
 * Schema Complex Types and Top-Level Elements                                *
 *   http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01                    *
 *                                                                            *
\******************************************************************************/


/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":HelloType is a complexType.
struct wsdd__HelloType
{
/// Imported element reference "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReference.
    _wsa__EndpointReference              wsa__EndpointReference         1;	///< Required element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Scopes.
    struct wsdd__ScopesType*             Scopes                         0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":XAddrs.
    wsdd__UriListType                    XAddrs                         0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":MetadataVersion.
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

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":ByeType is a complexType.
struct wsdd__ByeType
{
/// Imported element reference "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReference.
    _wsa__EndpointReference              wsa__EndpointReference         1;	///< Required element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Scopes.
    struct wsdd__ScopesType*             Scopes                         0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":XAddrs.
    wsdd__UriListType                    XAddrs                         0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":MetadataVersion.
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

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":ProbeType is a complexType.
struct wsdd__ProbeType
{
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Scopes.
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

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":ProbeMatchesType is a complexType.
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

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":ProbeMatchType is a complexType.
struct wsdd__ProbeMatchType
{
/// Imported element reference "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReference.
    _wsa__EndpointReference              wsa__EndpointReference         1;	///< Required element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Scopes.
    struct wsdd__ScopesType*             Scopes                         0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":XAddrs.
    wsdd__UriListType                    XAddrs                         0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":MetadataVersion.
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

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":ResolveType is a complexType.
struct wsdd__ResolveType
{
/// Imported element reference "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReference.
    _wsa__EndpointReference              wsa__EndpointReference         1;	///< Required element.
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

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":ResolveMatchesType is a complexType.
struct wsdd__ResolveMatchesType
{
/// Element ResolveMatch of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":ResolveMatchType.
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

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":ResolveMatchType is a complexType.
struct wsdd__ResolveMatchType
{
/// Imported element reference "http://schemas.xmlsoap.org/ws/2004/08/addressing":EndpointReference.
    _wsa__EndpointReference              wsa__EndpointReference         1;	///< Required element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Types.
    wsdd__QNameListType                  Types                          0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Scopes.
    struct wsdd__ScopesType*             Scopes                         0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":XAddrs.
    wsdd__UriListType                    XAddrs                         0;	///< Optional element.
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":MetadataVersion.
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

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":SecurityType is a complexType.
struct wsdd__SecurityType
{
/// Element reference "http://schemas.xmlsoap.org/ws/2005/04/discovery":Sig.
    struct wsdd__SigType*                Sig                            0;	///< Optional element.
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":SigType is a complexType.
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

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":ScopesType is a complexType with simpleContent.
struct wsdd__ScopesType
{
/// __item wraps '"http://schemas.xmlsoap.org/ws/2005/04/discovery":UriListType' simpleContent.
    wsdd__UriListType                    __item                        ;
/// Attribute MatchBy of type xs:anyURI.
   @char*                                MatchBy                        0;	///< Optional attribute.
/// <anyAttribute namespace="##other">
/// TODO: Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// "http://schemas.xmlsoap.org/ws/2005/04/discovery":AppSequenceType is a complexType with complexContent restriction of xs:anyType.
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


/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":Hello of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":HelloType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":Bye of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":ByeType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":Probe of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":ProbeType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":ProbeMatches of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":ProbeMatchesType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":Resolve of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":ResolveType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":ResolveMatches of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":ResolveMatchesType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":Types of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":QNameListType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":Scopes of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":ScopesType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":XAddrs of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":UriListType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":MetadataVersion of type xs:unsignedInt.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":SupportedMatchingRules of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":UriListType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":Security of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":SecurityType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":Sig of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":SigType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// Top-level root element "http://schemas.xmlsoap.org/ws/2005/04/discovery":AppSequence of type "http://schemas.xmlsoap.org/ws/2005/04/discovery":AppSequenceType.
/// Note: use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/******************************************************************************\
 *                                                                            *
 * Additional Top-Level Attributes                                            *
 *   http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01                    *
 *                                                                            *
\******************************************************************************/


/// Top-level attribute "http://schemas.xmlsoap.org/ws/2005/04/discovery":Id of simpleType xs:ID.
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


@section wsdd Top-level root elements of schema "http://schemas.xmlsoap.org/ws/2005/04/discovery"

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
