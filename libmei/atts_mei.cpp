/////////////////////////////////////////////////////////////////////////////
// Authors:     Laurent Pugin and Rodolfo Zitellini
// Created:     2014
// Copyright (c) Authors and others. All rights reserved.
//
// Code generated using a modified version of libmei
// by Andrew Hankinson, Alastair Porter, and Others
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// NOTE: this file was generated with the Verovio libmei version and
// should not be edited because changes will be lost.
/////////////////////////////////////////////////////////////////////////////

#include "atts_mei.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "object.h"

/* #include_block */

namespace vrv {

//----------------------------------------------------------------------------
// AttNotationtype
//----------------------------------------------------------------------------

AttNotationtype::AttNotationtype() : Att()
{
    ResetNotationtype();
}

AttNotationtype::~AttNotationtype()
{
}

void AttNotationtype::ResetNotationtype()
{
    m_notationtype = NOTATIONTYPE_NONE;
    m_notationsubtype = "";
}

bool AttNotationtype::ReadNotationtype(pugi::xml_node element)
{
    bool hasAttribute = false;
    if (element.attribute("notationtype")) {
        this->SetNotationtype(StrToNotationtype(element.attribute("notationtype").value()));
        element.remove_attribute("notationtype");
        hasAttribute = true;
    }
    if (element.attribute("notationsubtype")) {
        this->SetNotationsubtype(StrToStr(element.attribute("notationsubtype").value()));
        element.remove_attribute("notationsubtype");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttNotationtype::WriteNotationtype(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasNotationtype()) {
        element.append_attribute("notationtype") = NotationtypeToStr(this->GetNotationtype()).c_str();
        wroteAttribute = true;
    }
    if (this->HasNotationsubtype()) {
        element.append_attribute("notationsubtype") = StrToStr(this->GetNotationsubtype()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttNotationtype::HasNotationtype() const
{
    return (m_notationtype != NOTATIONTYPE_NONE);
}

bool AttNotationtype::HasNotationsubtype() const
{
    return (m_notationsubtype != "");
}

/* include <attnotationsubtype> */

bool Att::SetMei(Object *element, std::string attrType, std::string attrValue)
{
    if (element->HasAttClass(ATT_NOTATIONTYPE)) {
        AttNotationtype *att = dynamic_cast<AttNotationtype *>(element);
        assert(att);
        if (attrType == "notationtype") {
            att->SetNotationtype(att->StrToNotationtype(attrValue));
            return true;
        }
        if (attrType == "notationsubtype") {
            att->SetNotationsubtype(att->StrToStr(attrValue));
            return true;
        }
    }

    return false;
}

void Att::GetMei(const Object *element, ArrayOfStrAttr *attributes)
{
    if (element->HasAttClass(ATT_NOTATIONTYPE)) {
        const AttNotationtype *att = dynamic_cast<const AttNotationtype *>(element);
        assert(att);
        if (att->HasNotationtype()) {
            attributes->push_back(std::make_pair("notationtype", att->NotationtypeToStr(att->GetNotationtype())));
        }
        if (att->HasNotationsubtype()) {
            attributes->push_back(std::make_pair("notationsubtype", att->StrToStr(att->GetNotationsubtype())));
        }
    }
}

} // vrv namespace
