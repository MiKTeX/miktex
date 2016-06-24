/*
	saml1.h

	Generated with:
	wsdl2h -cuyx -o saml1.h -t WS/WS-typemap.dat WS/oasis-sstc-saml-schema-assertion-1.1.xsd

	- Removed //gsoapopt
	- Changed //gsoap saml1 schema namespace directive to import directive

*/


/******************************************************************************\
 *                                                                            *
 * Definitions                                                                *
 *   urn:oasis:names:tc:SAML:1.0:assertion                                    *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Import                                                                     *
 *                                                                            *
\******************************************************************************/

#import "ds.h"	// ds = <http://www.w3.org/2000/09/xmldsig#>

/******************************************************************************\
 *                                                                            *
 * Schema Namespaces                                                          *
 *                                                                            *
\******************************************************************************/


/// <PRE><BLOCKQUOTE>
///   Document identifier: oasis-sstc-saml-schema-assertion-1.1
///   Location: http://www.oasis-open.org/committees/documents.php?wg_abbrev=security
///   Revision history:
///   V1.0 (November, 2002):
///   Initial standard schema.
///   V1.1 (September, 2003):
///   * Note that V1.1 of this schema has the same XML namespace as V1.0.
///   Rebased ID content directly on XML Schema types
///   Added DoNotCacheCondition element and DoNotCacheConditionType
/// </BLOCKQUOTE></PRE>
#define SOAP_NAMESPACE_OF_saml1	"urn:oasis:names:tc:SAML:1.0:assertion"
//gsoap saml1 schema import:	urn:oasis:names:tc:SAML:1.0:assertion
//gsoap saml1 schema form:	unqualified

/******************************************************************************\
 *                                                                            *
 * Built-in Schema Types and Top-Level Elements and Attributes                *
 *                                                                            *
\******************************************************************************/

// Imported element ""http://www.w3.org/2000/09/xmldsig#":KeyInfo" declared as _ds__KeyInfo.
// Imported element ""http://www.w3.org/2000/09/xmldsig#":Signature" declared as _ds__Signature.

/// @brief Typedef synonym for struct saml1__AssertionType.
typedef struct saml1__AssertionType saml1__AssertionType;
/// @brief Typedef synonym for struct saml1__ConditionsType.
typedef struct saml1__ConditionsType saml1__ConditionsType;
/// @brief Typedef synonym for struct saml1__ConditionAbstractType.
typedef struct saml1__ConditionAbstractType saml1__ConditionAbstractType;
/// @brief Typedef synonym for struct saml1__AudienceRestrictionConditionType.
typedef struct saml1__AudienceRestrictionConditionType saml1__AudienceRestrictionConditionType;
/// @brief Typedef synonym for struct saml1__DoNotCacheConditionType.
typedef struct saml1__DoNotCacheConditionType saml1__DoNotCacheConditionType;
/// @brief Typedef synonym for struct saml1__AdviceType.
typedef struct saml1__AdviceType saml1__AdviceType;
/// @brief Typedef synonym for struct saml1__StatementAbstractType.
typedef struct saml1__StatementAbstractType saml1__StatementAbstractType;
/// @brief Typedef synonym for struct saml1__SubjectStatementAbstractType.
typedef struct saml1__SubjectStatementAbstractType saml1__SubjectStatementAbstractType;
/// @brief Typedef synonym for struct saml1__SubjectType.
typedef struct saml1__SubjectType saml1__SubjectType;
/// @brief Typedef synonym for struct saml1__NameIdentifierType.
typedef struct saml1__NameIdentifierType saml1__NameIdentifierType;
/// @brief Typedef synonym for struct saml1__SubjectConfirmationType.
typedef struct saml1__SubjectConfirmationType saml1__SubjectConfirmationType;
/// @brief Typedef synonym for struct saml1__AuthenticationStatementType.
typedef struct saml1__AuthenticationStatementType saml1__AuthenticationStatementType;
/// @brief Typedef synonym for struct saml1__SubjectLocalityType.
typedef struct saml1__SubjectLocalityType saml1__SubjectLocalityType;
/// @brief Typedef synonym for struct saml1__AuthorityBindingType.
typedef struct saml1__AuthorityBindingType saml1__AuthorityBindingType;
/// @brief Typedef synonym for struct saml1__AuthorizationDecisionStatementType.
typedef struct saml1__AuthorizationDecisionStatementType saml1__AuthorizationDecisionStatementType;
/// @brief Typedef synonym for struct saml1__ActionType.
typedef struct saml1__ActionType saml1__ActionType;
/// @brief Typedef synonym for struct saml1__EvidenceType.
typedef struct saml1__EvidenceType saml1__EvidenceType;
/// @brief Typedef synonym for struct saml1__AttributeStatementType.
typedef struct saml1__AttributeStatementType saml1__AttributeStatementType;
/// @brief Typedef synonym for struct saml1__AttributeDesignatorType.
typedef struct saml1__AttributeDesignatorType saml1__AttributeDesignatorType;
/// @brief Typedef synonym for struct saml1__AttributeType.
typedef struct saml1__AttributeType saml1__AttributeType;

/******************************************************************************\
 *                                                                            *
 * Schema Types and Top-Level Elements and Attributes                         *
 *   urn:oasis:names:tc:SAML:1.0:assertion                                    *
 *                                                                            *
\******************************************************************************/

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":DecisionType is a simpleType restriction of XSD type xs:string.
///
/// @note The enum values are prefixed with "saml1__DecisionType__" to avoid name clashes, please use wsdl2h option -e to omit this prefix
enum saml1__DecisionType
{
	saml1__DecisionType__Permit,	///< xs:string value="Permit"
	saml1__DecisionType__Deny,	///< xs:string value="Deny"
	saml1__DecisionType__Indeterminate,	///< xs:string value="Indeterminate"
};

/// @brief Typedef synonym for enum saml1__DecisionType.
typedef enum saml1__DecisionType saml1__DecisionType;

/******************************************************************************\
 *                                                                            *
 * Schema Complex Types and Top-Level Elements                                *
 *   urn:oasis:names:tc:SAML:1.0:assertion                                    *
 *                                                                            *
\******************************************************************************/

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":AssertionType is a complexType.
///
struct saml1__AssertionType
{
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Conditions.
    struct saml1__ConditionsType*        saml1__Conditions              0;	///< Optional element.
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Advice.
    struct saml1__AdviceType*            saml1__Advice                  0;	///< Optional element.
//  BEGIN CHOICE <xs:choice maxOccurs="unbounded">
   $int                                  __size_AssertionType           0;
    struct __saml1__union_AssertionType
    {
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Statement.
    struct saml1__StatementAbstractType*  saml1__Statement              ;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":SubjectStatement.
    struct saml1__SubjectStatementAbstractType*  saml1__SubjectStatement       ;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":AuthenticationStatement.
    struct saml1__AuthenticationStatementType*  saml1__AuthenticationStatement;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":AuthorizationDecisionStatement.
    struct saml1__AuthorizationDecisionStatementType*  saml1__AuthorizationDecisionStatement;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":AttributeStatement.
    struct saml1__AttributeStatementType*  saml1__AttributeStatement     ;
    }                                   *__union_AssertionType         ;
//  END OF CHOICE
/// Imported element reference "http://www.w3.org/2000/09/xmldsig#":Signature.
    _ds__Signature*                      ds__Signature                  0;	///< Optional element.
/// Attribute "MajorVersion" of XSD type xs:integer.
   @char*                                MajorVersion                   1;	///< Required attribute.
/// Attribute "MinorVersion" of XSD type xs:integer.
   @char*                                MinorVersion                   1;	///< Required attribute.
/// Attribute "AssertionID" of XSD type xs:ID.
   @char*                                AssertionID                    1;	///< Required attribute.
/// Attribute "Issuer" of XSD type xs:string.
   @char*                                Issuer                         1;	///< Required attribute.
/// Attribute "IssueInstant" of XSD type xs:dateTime.
   @time_t                               IssueInstant                   1;	///< Required attribute.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":ConditionsType is a complexType.
///
struct saml1__ConditionsType
{
//  BEGIN CHOICE <xs:choice minOccurs="0" maxOccurs="unbounded">
   $int                                  __size_ConditionsType          0;
    struct __saml1__union_ConditionsType
    {
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":AudienceRestrictionCondition.
    struct saml1__AudienceRestrictionConditionType*  saml1__AudienceRestrictionCondition;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":DoNotCacheCondition.
    struct saml1__DoNotCacheConditionType*  saml1__DoNotCacheCondition    ;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Condition.
    struct saml1__ConditionAbstractType*  saml1__Condition              ;
    }                                   *__union_ConditionsType        ;
//  END OF CHOICE
/// Attribute "NotBefore" of XSD type xs:dateTime.
   @time_t*                              NotBefore                      0;	///< Optional attribute.
/// Attribute "NotOnOrAfter" of XSD type xs:dateTime.
   @time_t*                              NotOnOrAfter                   0;	///< Optional attribute.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":ConditionAbstractType is an abstract complexType.
///
struct saml1__ConditionAbstractType
{
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":AdviceType is a complexType.
///
struct saml1__AdviceType
{
//  BEGIN CHOICE <xs:choice minOccurs="0" maxOccurs="unbounded">
   $int                                  __size_AdviceType              0;
    struct __saml1__union_AdviceType
    {
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":AssertionIDReference.
    char*                                saml1__AssertionIDReference   ;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Assertion.
    struct saml1__AssertionType*         saml1__Assertion              ;
/// @todo <any namespace="##other">
/// @todo Schema extensibility is user-definable.
///       Consult the protocol documentation to change or insert declarations.
///       Use wsdl2h option -x to remove this element.
///       Use wsdl2h option -d for xsd__anyType DOM (soap_dom_element).
    }                                   *__union_AdviceType            ;
//  END OF CHOICE
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":StatementAbstractType is an abstract complexType.
///
struct saml1__StatementAbstractType
{
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":SubjectType is a complexType.
///
struct saml1__SubjectType
{
//  BEGIN CHOICE <xs:choice>
/// @note <xs:choice> with embedded <xs:sequence> or <xs:group> prevents the use of a union for <xs:choice>. Instead of being members of a union, the following members are declared optional. Only one member should be non-NULL by choice.
//  BEGIN SEQUENCE <xs:sequence>
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":NameIdentifier.
    struct saml1__NameIdentifierType*    saml1__NameIdentifier         ;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":SubjectConfirmation.
    struct saml1__SubjectConfirmationType*  saml1__SubjectConfirmation    ;
//  END OF SEQUENCE
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":SubjectConfirmation.
    struct saml1__SubjectConfirmationType*  saml1__SubjectConfirmation    ;
//  END OF CHOICE
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":SubjectConfirmationType is a complexType.
///
struct saml1__SubjectConfirmationType
{
/// Size of the dynamic array of char** is 1..unbounded.
   $int                                  __sizeConfirmationMethod       1;
/// Array char** of size 1..unbounded.
    char**                               saml1__ConfirmationMethod      1;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":SubjectConfirmationData.
    _XML                                 saml1__SubjectConfirmationData 0;	///< Optional element.
/// Imported element reference "http://www.w3.org/2000/09/xmldsig#":KeyInfo.
    _ds__KeyInfo*                        ds__KeyInfo                    0;	///< Optional element.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":SubjectLocalityType is a complexType.
///
struct saml1__SubjectLocalityType
{
/// Attribute "IPAddress" of XSD type xs:string.
   @char*                                IPAddress                      0;	///< Optional attribute.
/// Attribute "DNSAddress" of XSD type xs:string.
   @char*                                DNSAddress                     0;	///< Optional attribute.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":AuthorityBindingType is a complexType.
///
struct saml1__AuthorityBindingType
{
/// Attribute "AuthorityKind" of XSD type xs:QName.
   @_QName                               AuthorityKind                  1;	///< Required attribute.
/// Attribute "Location" of XSD type xs:anyURI.
   @char*                                Location                       1;	///< Required attribute.
/// Attribute "Binding" of XSD type xs:anyURI.
   @char*                                Binding                        1;	///< Required attribute.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":EvidenceType is a complexType.
///
struct saml1__EvidenceType
{
//  BEGIN CHOICE <xs:choice maxOccurs="unbounded">
   $int                                  __size_EvidenceType            0;
    struct __saml1__union_EvidenceType
    {
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":AssertionIDReference.
    char*                                saml1__AssertionIDReference   ;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Assertion.
    struct saml1__AssertionType*         saml1__Assertion              ;
    }                                   *__union_EvidenceType          ;
//  END OF CHOICE
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":AttributeDesignatorType is a complexType.
///
struct saml1__AttributeDesignatorType
{
/// Attribute "AttributeName" of XSD type xs:string.
   @char*                                AttributeName                  1;	///< Required attribute.
/// Attribute "AttributeNamespace" of XSD type xs:anyURI.
   @char*                                AttributeNamespace             1;	///< Required attribute.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":AudienceRestrictionConditionType is a complexType with complexContent extension of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":ConditionAbstractType.
///
struct saml1__AudienceRestrictionConditionType
{
/// INHERITED FROM saml1__ConditionAbstractType:
//  END OF INHERITED FROM saml1__ConditionAbstractType
/// Size of the dynamic array of char** is 1..unbounded.
   $int                                  __sizeAudience                 1;
/// Array char** of size 1..unbounded.
    char**                               saml1__Audience                1;
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":DoNotCacheConditionType is a complexType with complexContent extension of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":ConditionAbstractType.
///
struct saml1__DoNotCacheConditionType
{
/// INHERITED FROM saml1__ConditionAbstractType:
//  END OF INHERITED FROM saml1__ConditionAbstractType
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":SubjectStatementAbstractType is an abstract complexType with complexContent extension of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":StatementAbstractType.
///
struct saml1__SubjectStatementAbstractType
{
/// INHERITED FROM saml1__StatementAbstractType:
//  END OF INHERITED FROM saml1__StatementAbstractType
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Subject.
    struct saml1__SubjectType*           saml1__Subject                 1;	///< Required element.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":NameIdentifierType is a complexType with simpleContent.
///
struct saml1__NameIdentifierType
{
/// __item wraps "xs:string" simpleContent.
    char*                                __item                        ;
/// Attribute "NameQualifier" of XSD type xs:string.
   @char*                                NameQualifier                  0;	///< Optional attribute.
/// Attribute "Format" of XSD type xs:anyURI.
   @char*                                Format                         0;	///< Optional attribute.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":ActionType is a complexType with simpleContent.
///
struct saml1__ActionType
{
/// __item wraps "xs:string" simpleContent.
    char*                                __item                        ;
/// Attribute "Namespace" of XSD type xs:anyURI.
   @char*                                Namespace                      0;	///< Optional attribute.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":AttributeType is a complexType with complexContent extension of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AttributeDesignatorType.
///
struct saml1__AttributeType
{
/// INHERITED FROM saml1__AttributeDesignatorType:
/// Attribute "AttributeName" of XSD type xs:string.
   @char*                                AttributeName                  1;	///< Required attribute.
/// Attribute "AttributeNamespace" of XSD type xs:anyURI.
   @char*                                AttributeNamespace             1;	///< Required attribute.
//  END OF INHERITED FROM saml1__AttributeDesignatorType
/// Size of the dynamic array of _XML* is 1..unbounded.
   $int                                  __sizeAttributeValue           1;
/// Array _XML* of size 1..unbounded.
    _XML*                                saml1__AttributeValue          1;
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":AuthenticationStatementType is a complexType with complexContent extension of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":SubjectStatementAbstractType.
///
struct saml1__AuthenticationStatementType
{
/// INHERITED FROM saml1__StatementAbstractType:
//  END OF INHERITED FROM saml1__StatementAbstractType
/// INHERITED FROM saml1__SubjectStatementAbstractType:
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Subject.
    struct saml1__SubjectType*           saml1__Subject                 1;	///< Required element.
//  END OF INHERITED FROM saml1__SubjectStatementAbstractType
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":SubjectLocality.
    struct saml1__SubjectLocalityType*   saml1__SubjectLocality         0;	///< Optional element.
/// Size of the dynamic array of struct saml1__AuthorityBindingType* is 0..unbounded.
   $int                                  __sizeAuthorityBinding         0;
/// Array struct saml1__AuthorityBindingType* of size 0..unbounded.
    struct saml1__AuthorityBindingType*  saml1__AuthorityBinding        0;
/// Attribute "AuthenticationMethod" of XSD type xs:anyURI.
   @char*                                AuthenticationMethod           1;	///< Required attribute.
/// Attribute "AuthenticationInstant" of XSD type xs:dateTime.
   @time_t                               AuthenticationInstant          1;	///< Required attribute.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":AuthorizationDecisionStatementType is a complexType with complexContent extension of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":SubjectStatementAbstractType.
///
struct saml1__AuthorizationDecisionStatementType
{
/// INHERITED FROM saml1__StatementAbstractType:
//  END OF INHERITED FROM saml1__StatementAbstractType
/// INHERITED FROM saml1__SubjectStatementAbstractType:
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Subject.
    struct saml1__SubjectType*           saml1__Subject                 1;	///< Required element.
//  END OF INHERITED FROM saml1__SubjectStatementAbstractType
/// Size of the dynamic array of struct saml1__ActionType* is 1..unbounded.
   $int                                  __sizeAction                   1;
/// Array struct saml1__ActionType* of size 1..unbounded.
    struct saml1__ActionType*            saml1__Action                  1;
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Evidence.
    struct saml1__EvidenceType*          saml1__Evidence                0;	///< Optional element.
/// Attribute "Resource" of XSD type xs:anyURI.
   @char*                                Resource                       1;	///< Required attribute.
/// Attribute "Decision" of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":DecisionType.
   @enum saml1__DecisionType             Decision                       1;	///< Required attribute.
};

/// @brief "urn:oasis:names:tc:SAML:1.0:assertion":AttributeStatementType is a complexType with complexContent extension of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":SubjectStatementAbstractType.
///
struct saml1__AttributeStatementType
{
/// INHERITED FROM saml1__StatementAbstractType:
//  END OF INHERITED FROM saml1__StatementAbstractType
/// INHERITED FROM saml1__SubjectStatementAbstractType:
/// Element reference "urn:oasis:names:tc:SAML:1.0:assertion:""urn:oasis:names:tc:SAML:1.0:assertion":Subject.
    struct saml1__SubjectType*           saml1__Subject                 1;	///< Required element.
//  END OF INHERITED FROM saml1__SubjectStatementAbstractType
/// Size of the dynamic array of struct saml1__AttributeType* is 1..unbounded.
   $int                                  __sizeAttribute                1;
/// Array struct saml1__AttributeType* of size 1..unbounded.
    struct saml1__AttributeType*         saml1__Attribute               1;
};


/******************************************************************************\
 *                                                                            *
 * Additional Top-Level Elements                                              *
 *   urn:oasis:names:tc:SAML:1.0:assertion                                    *
 *                                                                            *
\******************************************************************************/

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":AssertionIDReference of XSD type xs:NCName.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Assertion of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AssertionType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Conditions of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":ConditionsType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Condition of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":ConditionAbstractType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":AudienceRestrictionCondition of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AudienceRestrictionConditionType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Audience of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":DoNotCacheCondition of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":DoNotCacheConditionType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Advice of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AdviceType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Statement of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":StatementAbstractType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":SubjectStatement of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":SubjectStatementAbstractType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Subject of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":SubjectType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":NameIdentifier of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":NameIdentifierType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":SubjectConfirmation of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":SubjectConfirmationType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":SubjectConfirmationData of XSD type xs:anyType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":ConfirmationMethod of XSD type xs:anyURI.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":AuthenticationStatement of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AuthenticationStatementType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":SubjectLocality of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":SubjectLocalityType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":AuthorityBinding of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AuthorityBindingType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":AuthorizationDecisionStatement of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AuthorizationDecisionStatementType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Action of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":ActionType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Evidence of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":EvidenceType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":AttributeStatement of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AttributeStatementType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":AttributeDesignator of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AttributeDesignatorType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":Attribute of XSD type "urn:oasis:names:tc:SAML:1.0:assertion":AttributeType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.

/// @brief Top-level root element "urn:oasis:names:tc:SAML:1.0:assertion":AttributeValue of XSD type xs:anyType.
/// @note Use wsdl2h option -g to auto-generate a top-level root element declaration and processing code.


/******************************************************************************\
 *                                                                            *
 * Additional Top-Level Attributes                                            *
 *   urn:oasis:names:tc:SAML:1.0:assertion                                    *
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


@section saml1 Top-level root elements of schema "urn:oasis:names:tc:SAML:1.0:assertion"

  - <AssertionIDReference> (use wsdl2h option -g to auto-generate)

  - <Assertion> (use wsdl2h option -g to auto-generate)

  - <Conditions> (use wsdl2h option -g to auto-generate)

  - <Condition> (use wsdl2h option -g to auto-generate)

  - <AudienceRestrictionCondition> (use wsdl2h option -g to auto-generate)

  - <Audience> (use wsdl2h option -g to auto-generate)

  - <DoNotCacheCondition> (use wsdl2h option -g to auto-generate)

  - <Advice> (use wsdl2h option -g to auto-generate)

  - <Statement> (use wsdl2h option -g to auto-generate)

  - <SubjectStatement> (use wsdl2h option -g to auto-generate)

  - <Subject> (use wsdl2h option -g to auto-generate)

  - <NameIdentifier> (use wsdl2h option -g to auto-generate)

  - <SubjectConfirmation> (use wsdl2h option -g to auto-generate)

  - <SubjectConfirmationData> (use wsdl2h option -g to auto-generate)

  - <ConfirmationMethod> (use wsdl2h option -g to auto-generate)

  - <AuthenticationStatement> (use wsdl2h option -g to auto-generate)

  - <SubjectLocality> (use wsdl2h option -g to auto-generate)

  - <AuthorityBinding> (use wsdl2h option -g to auto-generate)

  - <AuthorizationDecisionStatement> (use wsdl2h option -g to auto-generate)

  - <Action> (use wsdl2h option -g to auto-generate)

  - <Evidence> (use wsdl2h option -g to auto-generate)

  - <AttributeStatement> (use wsdl2h option -g to auto-generate)

  - <AttributeDesignator> (use wsdl2h option -g to auto-generate)

  - <Attribute> (use wsdl2h option -g to auto-generate)

  - <AttributeValue> (use wsdl2h option -g to auto-generate)

*/

/* End of saml1.h */
