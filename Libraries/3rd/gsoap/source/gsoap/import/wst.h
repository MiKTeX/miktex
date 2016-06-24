/*
	wst.h

	WS-Trust 2005/12
	Generated with:
	wsdl2h -cuyx -o wst.h -t WS/WS-typemap.dat WS/WS-Trust.xsd

	- Removed //gsoapopt
	- Changed http://docs.oasis-open.org/ws-sx/ws-trust/200512 to remove trailing /
	- Changed //gsoap wst schema namespace directive to import directive
	- Added #import "wstx.h" at the end of these definitions

*/

/******************************************************************************\
 *                                                                            *
 * Definitions                                                                *
 *   http://docs.oasis-open.org/ws-sx/ws-trust/200512/                        *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Import                                                                     *
 *                                                                            *
\******************************************************************************/

#import "wsse.h"	// wsse = <http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd>
#import "wsu.h"	// wsu = <http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd>
#import "wsa5.h"	// wsa5 = <http://www.w3.org/2005/08/addressing>

/******************************************************************************\
 *                                                                            *
 * Schema Namespaces                                                          *
 *                                                                            *
\******************************************************************************/

#define SOAP_NAMESPACE_OF_wst	"http://docs.oasis-open.org/ws-sx/ws-trust/200512"
//gsoap wst   schema import:	http://docs.oasis-open.org/ws-sx/ws-trust/200512
//gsoap wst   schema elementForm:	qualified
//gsoap wst   schema attributeForm:	unqualified

/******************************************************************************\
 *                                                                            *
 * Built-in Schema Types and Top-Level Elements and Attributes                *
 *                                                                            *
\******************************************************************************/

// Imported XSD type ""http://www.w3.org/2005/08/addressing":EndpointReferenceType" defined by wsa5__EndpointReferenceType.
/// Imported element ""http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":SecurityTokenReference" from typemap "/Users/engelen/Projects/gsoap/WS/WS-typemap.dat".

/// @brief Typedef synonym for struct wst__RequestSecurityTokenType.
typedef struct wst__RequestSecurityTokenType wst__RequestSecurityTokenType;
/// @brief Typedef synonym for struct wst__RequestSecurityTokenResponseType.
typedef struct wst__RequestSecurityTokenResponseType wst__RequestSecurityTokenResponseType;
/// Imported complexType "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedSecurityTokenType from typemap "/Users/engelen/Projects/gsoap/WS/WS-typemap.dat".
#import "saml1.h"
typedef struct wst__RequestedSecurityTokenType
{	saml1__AssertionType *saml1__Assertion;
        _wsse__SecurityTokenReference *wsse__SecurityTokenReference;
} wst__RequestedSecurityTokenType;

/// @brief Typedef synonym for struct wst__BinarySecretType.
typedef struct wst__BinarySecretType wst__BinarySecretType;
/// @brief Typedef synonym for struct wst__ClaimsType.
typedef struct wst__ClaimsType wst__ClaimsType;
/// Imported complexType "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":EntropyType from typemap "/Users/engelen/Projects/gsoap/WS/WS-typemap.dat".
typedef struct wst__EntropyType
{	struct wst__BinarySecretType *BinarySecret;
} wst__EntropyType;

/// @brief Typedef synonym for struct wst__LifetimeType.
typedef struct wst__LifetimeType wst__LifetimeType;
/// @brief Typedef synonym for struct wst__RequestSecurityTokenCollectionType.
typedef struct wst__RequestSecurityTokenCollectionType wst__RequestSecurityTokenCollectionType;
/// @brief Typedef synonym for struct wst__RequestSecurityTokenResponseCollectionType.
typedef struct wst__RequestSecurityTokenResponseCollectionType wst__RequestSecurityTokenResponseCollectionType;
/// @brief Typedef synonym for struct wst__RequestedReferenceType.
typedef struct wst__RequestedReferenceType wst__RequestedReferenceType;
/// @brief Typedef synonym for struct wst__RequestedProofTokenType.
typedef struct wst__RequestedProofTokenType wst__RequestedProofTokenType;
/// @brief Typedef synonym for struct wst__RenewTargetType.
typedef struct wst__RenewTargetType wst__RenewTargetType;
/// @brief Typedef synonym for struct wst__AllowPostdatingType.
typedef struct wst__AllowPostdatingType wst__AllowPostdatingType;
/// @brief Typedef synonym for struct wst__RenewingType.
typedef struct wst__RenewingType wst__RenewingType;
/// @brief Typedef synonym for struct wst__CancelTargetType.
typedef struct wst__CancelTargetType wst__CancelTargetType;
/// @brief Typedef synonym for struct wst__RequestedTokenCancelledType.
typedef struct wst__RequestedTokenCancelledType wst__RequestedTokenCancelledType;
/// @brief Typedef synonym for struct wst__ValidateTargetType.
typedef struct wst__ValidateTargetType wst__ValidateTargetType;
/// @brief Typedef synonym for struct wst__StatusType.
typedef struct wst__StatusType wst__StatusType;
/// @brief Typedef synonym for struct wst__SignChallengeType.
typedef struct wst__SignChallengeType wst__SignChallengeType;
/// @brief Typedef synonym for struct wst__BinaryExchangeType.
typedef struct wst__BinaryExchangeType wst__BinaryExchangeType;
/// @brief Typedef synonym for struct wst__RequestKETType.
typedef struct wst__RequestKETType wst__RequestKETType;
/// @brief Typedef synonym for struct wst__KeyExchangeTokenType.
typedef struct wst__KeyExchangeTokenType wst__KeyExchangeTokenType;
/// @brief Typedef synonym for struct wst__AuthenticatorType.
typedef struct wst__AuthenticatorType wst__AuthenticatorType;
/// @brief Typedef synonym for struct wst__OnBehalfOfType.
typedef struct wst__OnBehalfOfType wst__OnBehalfOfType;
/// @brief Typedef synonym for struct wst__EncryptionType.
typedef struct wst__EncryptionType wst__EncryptionType;
/// @brief Typedef synonym for struct wst__ProofEncryptionType.
typedef struct wst__ProofEncryptionType wst__ProofEncryptionType;
/// @brief Typedef synonym for struct wst__UseKeyType.
typedef struct wst__UseKeyType wst__UseKeyType;
/// @brief Typedef synonym for struct wst__DelegateToType.
typedef struct wst__DelegateToType wst__DelegateToType;
/// @brief Typedef synonym for struct wst__ParticipantsType.
typedef struct wst__ParticipantsType wst__ParticipantsType;
/// @brief Typedef synonym for struct wst__ParticipantType.
typedef struct wst__ParticipantType wst__ParticipantType;

/******************************************************************************\
 *                                                                            *
 * Schema Types and Top-Level Elements and Attributes                         *
 *   http://docs.oasis-open.org/ws-sx/ws-trust/200512/                        *
 *                                                                            *
\******************************************************************************/

/// @brief Union of values from member types "wst:RequestTypeEnum xs:anyURI".
typedef char* wst__RequestTypeOpenEnum;

/// @brief Union of values from member types "wst:BinarySecretTypeEnum xs:anyURI".
typedef char* wst__BinarySecretTypeOpenEnum;

/// @brief Union of values from member types "wst:ComputedKeyEnum xs:anyURI".
typedef char* wst__ComputedKeyOpenEnum;

/// @brief Union of values from member types "wst:StatusCodeEnum xs:anyURI".
typedef char* wst__StatusCodeOpenEnum;

/// @brief Union of values from member types "wst:KeyTypeEnum xs:anyURI".
typedef char* wst__KeyTypeOpenEnum;

/// Imported simpleType "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestTypeEnum from typemap "/Users/engelen/Projects/gsoap/WS/WS-typemap.dat".
// simpleType definition intentionally left blank.
/// Imported simpleType "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":BinarySecretTypeEnum from typemap "/Users/engelen/Projects/gsoap/WS/WS-typemap.dat".
// simpleType definition intentionally left blank.
/// Imported simpleType "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ComputedKeyEnum from typemap "/Users/engelen/Projects/gsoap/WS/WS-typemap.dat".
// simpleType definition intentionally left blank.
/// Imported simpleType "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":StatusCodeEnum from typemap "/Users/engelen/Projects/gsoap/WS/WS-typemap.dat".
// simpleType definition intentionally left blank.
/// Imported simpleType "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":KeyTypeEnum from typemap "/Users/engelen/Projects/gsoap/WS/WS-typemap.dat".
// simpleType definition intentionally left blank.

/******************************************************************************\
 *                                                                            *
 * Schema Complex Types and Top-Level Elements                                *
 *   http://docs.oasis-open.org/ws-sx/ws-trust/200512/                        *
 *                                                                            *
\******************************************************************************/

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenType is a complexType.
///
/// <PRE><BLOCKQUOTE>
///   Actual content model is non-deterministic, hence wildcard. The following shows intended content model:
///   <xs:element ref='wst:TokenType' minOccurs='0' />
///   <xs:element ref='wst:RequestType' />
///   <xs:element ref='wsp:AppliesTo' minOccurs='0' />
///   <xs:element ref='wst:Claims' minOccurs='0' />
///   <xs:element ref='wst:Entropy' minOccurs='0' />
///   <xs:element ref='wst:Lifetime' minOccurs='0' />
///   <xs:element ref='wst:AllowPostdating' minOccurs='0' />
///   <xs:element ref='wst:Renewing' minOccurs='0' />
///   <xs:element ref='wst:OnBehalfOf' minOccurs='0' />
///   <xs:element ref='wst:Issuer' minOccurs='0' />
///   <xs:element ref='wst:AuthenticationType' minOccurs='0' />
///   <xs:element ref='wst:KeyType' minOccurs='0' />
///   <xs:element ref='wst:KeySize' minOccurs='0' />
///   <xs:element ref='wst:SignatureAlgorithm' minOccurs='0' />
///   <xs:element ref='wst:Encryption' minOccurs='0' />
///   <xs:element ref='wst:EncryptionAlgorithm' minOccurs='0' />
///   <xs:element ref='wst:CanonicalizationAlgorithm' minOccurs='0' />
///   <xs:element ref='wst:ProofEncryption' minOccurs='0' />
///   <xs:element ref='wst:UseKey' minOccurs='0' />
///   <xs:element ref='wst:SignWith' minOccurs='0' />
///   <xs:element ref='wst:EncryptWith' minOccurs='0' />
///   <xs:element ref='wst:DelegateTo' minOccurs='0' />
///   <xs:element ref='wst:Forwardable' minOccurs='0' />
///   <xs:element ref='wst:Delegatable' minOccurs='0' />
///   <xs:element ref='wsp:Policy' minOccurs='0' />
///   <xs:element ref='wsp:PolicyReference' minOccurs='0' />
///   <xs:any namespace='##other' processContents='lax' minOccurs='0' maxOccurs='unbounded' />
/// </BLOCKQUOTE></PRE>
struct wst__RequestSecurityTokenType
{
/// @todo <any namespace="##any" minOccurs="0" maxOccurs="unbounded">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// Attribute "Context" of XSD type xs:anyURI.
   @char*                                Context                        0;	///< Optional attribute.
/// @todo <anyAttribute namespace="##other">.
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
/// Member declared in /Users/engelen/Projects/gsoap/WS/WS-typemap.dat
       char*                                TokenType;
/// Member declared in /Users/engelen/Projects/gsoap/WS/WS-typemap.dat
       wst__RequestTypeOpenEnum             RequestType;
/// Member declared in /Users/engelen/Projects/gsoap/WS/WS-typemap.dat
       unsigned int*                        KeySize;
/// Member declared in /Users/engelen/Projects/gsoap/WS/WS-typemap.dat
       struct wst__BinaryExchangeType*      BinaryExhange;
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenResponseType is a complexType.
///
/// <PRE><BLOCKQUOTE>
///   Actual content model is non-deterministic, hence wildcard. The following shows intended content model:
///   <xs:element ref='wst:TokenType' minOccurs='0' />
///   <xs:element ref='wst:RequestType' />
///   <xs:element ref='wst:RequestedSecurityToken' minOccurs='0' />
///   <xs:element ref='wsp:AppliesTo' minOccurs='0' />
///   <xs:element ref='wst:RequestedAttachedReference' minOccurs='0' />
///   <xs:element ref='wst:RequestedUnattachedReference' minOccurs='0' />
///   <xs:element ref='wst:RequestedProofToken' minOccurs='0' />
///   <xs:element ref='wst:Entropy' minOccurs='0' />
///   <xs:element ref='wst:Lifetime' minOccurs='0' />
///   <xs:element ref='wst:Status' minOccurs='0' />
///   <xs:element ref='wst:AllowPostdating' minOccurs='0' />
///   <xs:element ref='wst:Renewing' minOccurs='0' />
///   <xs:element ref='wst:OnBehalfOf' minOccurs='0' />
///   <xs:element ref='wst:Issuer' minOccurs='0' />
///   <xs:element ref='wst:AuthenticationType' minOccurs='0' />
///   <xs:element ref='wst:Authenticator' minOccurs='0' />
///   <xs:element ref='wst:KeyType' minOccurs='0' />
///   <xs:element ref='wst:KeySize' minOccurs='0' />
///   <xs:element ref='wst:SignatureAlgorithm' minOccurs='0' />
///   <xs:element ref='wst:Encryption' minOccurs='0' />
///   <xs:element ref='wst:EncryptionAlgorithm' minOccurs='0' />
///   <xs:element ref='wst:CanonicalizationAlgorithm' minOccurs='0' />
///   <xs:element ref='wst:ProofEncryption' minOccurs='0' />
///   <xs:element ref='wst:UseKey' minOccurs='0' />
///   <xs:element ref='wst:SignWith' minOccurs='0' />
///   <xs:element ref='wst:EncryptWith' minOccurs='0' />
///   <xs:element ref='wst:DelegateTo' minOccurs='0' />
///   <xs:element ref='wst:Forwardable' minOccurs='0' />
///   <xs:element ref='wst:Delegatable' minOccurs='0' />
///   <xs:element ref='wsp:Policy' minOccurs='0' />
///   <xs:element ref='wsp:PolicyReference' minOccurs='0' />
///   <xs:any namespace='##other' processContents='lax' minOccurs='0' maxOccurs='unbounded' />
/// </BLOCKQUOTE></PRE>
struct wst__RequestSecurityTokenResponseType
{
/// @todo <any namespace="##any" minOccurs="0" maxOccurs="unbounded">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// Attribute "Context" of XSD type xs:anyURI.
   @char*                                Context                        0;	///< Optional attribute.
/// @todo <anyAttribute namespace="##other">.
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
/// Member declared in /Users/engelen/Projects/gsoap/WS/WS-typemap.dat
       char*                                TokenType;
/// Member declared in /Users/engelen/Projects/gsoap/WS/WS-typemap.dat
       struct wst__RequestedSecurityTokenType*  RequestedSecurityToken;
/// Member declared in /Users/engelen/Projects/gsoap/WS/WS-typemap.dat
       struct wst__BinaryExchangeType*      BinaryExhange;
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ClaimsType is a complexType.
///
struct wst__ClaimsType
{
/// @todo <any namespace="##any" minOccurs="0" maxOccurs="unbounded">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// Attribute "Dialect" of XSD type xs:anyURI.
   @char*                                Dialect                        0;	///< Optional attribute.
/// @todo <anyAttribute namespace="##other">.
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":LifetimeType is a complexType.
///
struct wst__LifetimeType
{
/// Imported element reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Created.
    char*                                wsu__Created                   0;	///< Optional element.
/// Imported element reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd":Expires.
    char*                                wsu__Expires                   0;	///< Optional element.
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenCollectionType is a complexType.
///
/// <PRE><BLOCKQUOTE>
///   The RequestSecurityTokenCollection (RSTC) element is used to provide multiple RST requests. One or more RSTR elements in an RSTRC element are returned in the response to the RequestSecurityTokenCollection.
/// </BLOCKQUOTE></PRE>
struct wst__RequestSecurityTokenCollectionType
{
/// Size of array of struct wst__RequestSecurityTokenType* is 2..unbounded.
   $int                                  __sizeRequestSecurityToken     2;
/// Array struct wst__RequestSecurityTokenType* of size 2..unbounded.
    struct wst__RequestSecurityTokenType*  RequestSecurityToken           2;
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenResponseCollectionType is a complexType.
///
/// <PRE><BLOCKQUOTE>
///   The <wst:RequestSecurityTokenResponseCollection> element (RSTRC) MUST be used to return a security token or response to a security token request on the final response.
/// </BLOCKQUOTE></PRE>
struct wst__RequestSecurityTokenResponseCollectionType
{
/// Size of the dynamic array of struct wst__RequestSecurityTokenResponseType* is 1..unbounded.
   $int                                  __sizeRequestSecurityTokenResponse 1;
/// Array struct wst__RequestSecurityTokenResponseType* of size 1..unbounded.
    struct wst__RequestSecurityTokenResponseType*  RequestSecurityTokenResponse   1;
/// @todo <anyAttribute namespace="##other">.
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedReferenceType is a complexType.
///
struct wst__RequestedReferenceType
{
/// Imported element reference "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd":SecurityTokenReference.
    _wsse__SecurityTokenReference        wsse__SecurityTokenReference   1;	///< Required element.
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedProofTokenType is a complexType.
///
struct wst__RequestedProofTokenType
{
/// @todo <any namespace="##any">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RenewTargetType is a complexType.
///
struct wst__RenewTargetType
{
/// @todo <any namespace="##other" minOccurs="1" maxOccurs="1">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":AllowPostdatingType is a complexType.
///
struct wst__AllowPostdatingType
{
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RenewingType is a complexType.
///
struct wst__RenewingType
{
/// Attribute "Allow" of XSD type xs:boolean.
   @char*                                Allow                          0;	///< Optional attribute.
/// Attribute "OK" of XSD type xs:boolean.
   @char*                                OK                             0;	///< Optional attribute.
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":CancelTargetType is a complexType.
///
struct wst__CancelTargetType
{
/// @todo <any namespace="##other" minOccurs="1" maxOccurs="1">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedTokenCancelledType is a complexType.
///
struct wst__RequestedTokenCancelledType
{
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ValidateTargetType is a complexType.
///
struct wst__ValidateTargetType
{
/// @todo <any namespace="##other" minOccurs="1" maxOccurs="1">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":StatusType is a complexType.
///
struct wst__StatusType
{
/// Element "Code" of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":StatusCodeOpenEnum.
    wst__StatusCodeOpenEnum              Code                           1;	///< Required element.
/// Element "Reason" of XSD type xs:string.
    char*                                Reason                         0;	///< Optional element.
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":SignChallengeType is a complexType.
///
struct wst__SignChallengeType
{
/// Element reference "http://docs.oasis-open.org/ws-sx/ws-trust/200512/:""http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Challenge.
    char*                                Challenge                      1;	///< Required element.
/// @todo <any namespace="##any" minOccurs="0" maxOccurs="unbounded">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// @todo <anyAttribute namespace="##any">.
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestKETType is a complexType.
///
struct wst__RequestKETType
{
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":KeyExchangeTokenType is a complexType.
///
struct wst__KeyExchangeTokenType
{
/// @todo <any namespace="##any" minOccurs="0" maxOccurs="unbounded">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":AuthenticatorType is a complexType.
///
struct wst__AuthenticatorType
{
/// Element reference "http://docs.oasis-open.org/ws-sx/ws-trust/200512/:""http://docs.oasis-open.org/ws-sx/ws-trust/200512/":CombinedHash.
    char*                                CombinedHash                   0;	///< Optional element.
/// @todo <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":OnBehalfOfType is a complexType.
///
struct wst__OnBehalfOfType
{
/// @todo <any namespace="##any">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":EncryptionType is a complexType.
///
struct wst__EncryptionType
{
/// @todo <any namespace="##any">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ProofEncryptionType is a complexType.
///
struct wst__ProofEncryptionType
{
/// @todo <any namespace="##any">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":UseKeyType is a complexType.
///
struct wst__UseKeyType
{
/// @todo <any namespace="##any" minOccurs="0">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
/// Attribute "Sig" of XSD type xs:anyURI.
   @char*                                Sig                            0;	///< Optional attribute.
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":DelegateToType is a complexType.
///
struct wst__DelegateToType
{
/// @todo <any namespace="##any">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ParticipantsType is a complexType.
///
struct wst__ParticipantsType
{
/// Element "Primary" of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ParticipantType.
    struct wst__ParticipantType*         Primary                        0;	///< Optional element.
/// Size of array of struct wst__ParticipantType* is 0..unbounded.
   $int                                  __sizeParticipant              0;
/// Array struct wst__ParticipantType* of size 0..unbounded.
    struct wst__ParticipantType*         Participant                    0;
/// @todo <any namespace="##other" minOccurs="0" maxOccurs="unbounded">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ParticipantType is a complexType.
///
struct wst__ParticipantType
{
/// @todo <any namespace="##any">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":BinarySecretType is a complexType with simpleContent.
///
struct wst__BinarySecretType
{
/// __item wraps "xs:base64Binary" simpleContent.
    char*                                __item                        ;
/// Attribute "Type" of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":BinarySecretTypeOpenEnum.
   @wst__BinarySecretTypeOpenEnum        Type                           0;	///< Optional attribute.
/// @todo <anyAttribute namespace="##other">.
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};

/// @brief "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":BinaryExchangeType is a complexType with simpleContent.
///
struct wst__BinaryExchangeType
{
/// __item wraps "xs:string" simpleContent.
    char*                                __item                        ;
/// Attribute "ValueType" of XSD type xs:anyURI.
   @char*                                ValueType                      1;	///< Required attribute.
/// Attribute "EncodingType" of XSD type xs:anyURI.
   @char*                                EncodingType                   1;	///< Required attribute.
/// @todo <anyAttribute namespace="##other">.
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this attribute.
///       Use wsdl2h option -d for xsd__anyAttribute DOM (soap_dom_attribute).
};


/******************************************************************************\
 *                                                                            *
 * Additional Top-Level Elements                                              *
 *   http://docs.oasis-open.org/ws-sx/ws-trust/200512/                        *
 *                                                                            *
\******************************************************************************/

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityToken of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":TokenType of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestType of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestTypeOpenEnum.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenResponse of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenResponseType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedSecurityToken of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedSecurityTokenType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":BinarySecret of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":BinarySecretType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Claims of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ClaimsType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Entropy of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":EntropyType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Lifetime of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":LifetimeType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenCollection of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenCollectionType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenResponseCollection of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenResponseCollectionType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ComputedKey of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ComputedKeyOpenEnum.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedAttachedReference of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedReferenceType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedUnattachedReference of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedReferenceType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedProofToken of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedProofTokenType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":IssuedTokens of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestSecurityTokenResponseCollectionType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RenewTarget of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RenewTargetType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":AllowPostdating of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":AllowPostdatingType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Renewing of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RenewingType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":CancelTarget of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":CancelTargetType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedTokenCancelled of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestedTokenCancelledType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ValidateTarget of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ValidateTargetType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Status of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":StatusType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":SignChallenge of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":SignChallengeType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":SignChallengeResponse of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":SignChallengeType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Challenge of XSD type xs:string.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":BinaryExchange of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":BinaryExchangeType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestKET of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":RequestKETType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":KeyExchangeToken of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":KeyExchangeTokenType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Authenticator of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":AuthenticatorType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":CombinedHash of XSD type xs:base64Binary.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":OnBehalfOf of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":OnBehalfOfType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Issuer of XSD type "http://www.w3.org/2005/08/addressing":EndpointReferenceType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":AuthenticationType of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":KeyType of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":KeyTypeOpenEnum.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":KeySize of XSD type xs:unsignedInt.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":SignatureAlgorithm of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":EncryptionAlgorithm of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":CanonicalizationAlgorithm of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ComputedKeyAlgorithm of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Encryption of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":EncryptionType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ProofEncryption of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ProofEncryptionType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":UseKey of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":UseKeyType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":KeyWrapAlgorithm of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":SignWith of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":EncryptWith of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":DelegateTo of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":DelegateToType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Forwardable of XSD type xs:boolean.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Delegatable of XSD type xs:boolean.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":Participants of XSD type "http://docs.oasis-open.org/ws-sx/ws-trust/200512/":ParticipantsType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.


/******************************************************************************\
 *                                                                            *
 * Additional Top-Level Attributes                                            *
 *   http://docs.oasis-open.org/ws-sx/ws-trust/200512/                        *
 *                                                                            *
\******************************************************************************/


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

Data can be read in XML and deserialized from:
  - a file descriptor, using soap->recvfd = fd
  - a socket, using soap->socket = ...
  - a C++ stream, using soap->is = ...
  - a buffer, using the soap->frecv() callback

Data can be serialized in XML and written to:
  - a file descriptor, using soap->sendfd = fd
  - a socket, using soap->socket = ...
  - a C++ stream, using soap->os = ...
  - a buffer, using the soap->fsend() callback

The following options are available for (de)serialization control:
  - soap->encodingStyle = NULL; to remove SOAP 1.1/1.2 encodingStyle
  - soap_mode(soap, SOAP_XML_TREE); XML without id-ref (no cycles!)
  - soap_mode(soap, SOAP_XML_GRAPH); XML with id-ref (including cycles)
  - soap_set_namespaces(soap, struct Namespace *nsmap); to set xmlns bindings


@section wst Top-level root elements of schema "http://docs.oasis-open.org/ws-sx/ws-trust/200512/"

  - <RequestSecurityToken> (use wsdl2h option -g to auto-generate)

  - <TokenType> (use wsdl2h option -g to auto-generate)

  - <RequestType> (use wsdl2h option -g to auto-generate)

  - <RequestSecurityTokenResponse> (use wsdl2h option -g to auto-generate)

  - <RequestedSecurityToken> (use wsdl2h option -g to auto-generate)

  - <BinarySecret> (use wsdl2h option -g to auto-generate)

  - <Claims> (use wsdl2h option -g to auto-generate)

  - <Entropy> (use wsdl2h option -g to auto-generate)

  - <Lifetime> (use wsdl2h option -g to auto-generate)

  - <RequestSecurityTokenCollection> (use wsdl2h option -g to auto-generate)

  - <RequestSecurityTokenResponseCollection> (use wsdl2h option -g to auto-generate)

  - <ComputedKey> (use wsdl2h option -g to auto-generate)

  - <RequestedAttachedReference> (use wsdl2h option -g to auto-generate)

  - <RequestedUnattachedReference> (use wsdl2h option -g to auto-generate)

  - <RequestedProofToken> (use wsdl2h option -g to auto-generate)

  - <IssuedTokens> (use wsdl2h option -g to auto-generate)

  - <RenewTarget> (use wsdl2h option -g to auto-generate)

  - <AllowPostdating> (use wsdl2h option -g to auto-generate)

  - <Renewing> (use wsdl2h option -g to auto-generate)

  - <CancelTarget> (use wsdl2h option -g to auto-generate)

  - <RequestedTokenCancelled> (use wsdl2h option -g to auto-generate)

  - <ValidateTarget> (use wsdl2h option -g to auto-generate)

  - <Status> (use wsdl2h option -g to auto-generate)

  - <SignChallenge> (use wsdl2h option -g to auto-generate)

  - <SignChallengeResponse> (use wsdl2h option -g to auto-generate)

  - <Challenge> (use wsdl2h option -g to auto-generate)

  - <BinaryExchange> (use wsdl2h option -g to auto-generate)

  - <RequestKET> (use wsdl2h option -g to auto-generate)

  - <KeyExchangeToken> (use wsdl2h option -g to auto-generate)

  - <Authenticator> (use wsdl2h option -g to auto-generate)

  - <CombinedHash> (use wsdl2h option -g to auto-generate)

  - <OnBehalfOf> (use wsdl2h option -g to auto-generate)

  - <Issuer> (use wsdl2h option -g to auto-generate)

  - <AuthenticationType> (use wsdl2h option -g to auto-generate)

  - <KeyType> (use wsdl2h option -g to auto-generate)

  - <KeySize> (use wsdl2h option -g to auto-generate)

  - <SignatureAlgorithm> (use wsdl2h option -g to auto-generate)

  - <EncryptionAlgorithm> (use wsdl2h option -g to auto-generate)

  - <CanonicalizationAlgorithm> (use wsdl2h option -g to auto-generate)

  - <ComputedKeyAlgorithm> (use wsdl2h option -g to auto-generate)

  - <Encryption> (use wsdl2h option -g to auto-generate)

  - <ProofEncryption> (use wsdl2h option -g to auto-generate)

  - <UseKey> (use wsdl2h option -g to auto-generate)

  - <KeyWrapAlgorithm> (use wsdl2h option -g to auto-generate)

  - <SignWith> (use wsdl2h option -g to auto-generate)

  - <EncryptWith> (use wsdl2h option -g to auto-generate)

  - <DelegateTo> (use wsdl2h option -g to auto-generate)

  - <Forwardable> (use wsdl2h option -g to auto-generate)

  - <Delegatable> (use wsdl2h option -g to auto-generate)

  - <Participants> (use wsdl2h option -g to auto-generate)

*/

#import "wstx.h"

/* End of wst.h */
