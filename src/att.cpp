/////////////////////////////////////////////////////////////////////////////
// Name:        att.cpp
// Author:      Laurent Pugin
// Created:     2014
// Copyright (c) Laurent Pugin. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "att.h"

//----------------------------------------------------------------------------

#include <sstream>
#include <stdlib.h>

//----------------------------------------------------------------------------

#include "object.h"
#include "vrv.h"

namespace vrv {

//----------------------------------------------------------------------------
// Att
//----------------------------------------------------------------------------

Att::Att() : AttConverter()
{
}

Att::~Att()
{
}

std::string Att::StrToStr(std::string str) const
{
    return str;
}

// Basic converters for writing

std::string Att::DblToStr(double data) const
{
    return StringFormat("%f", data);
}

std::string Att::IntToStr(int data) const
{
    return StringFormat("%d", data);
}

std::string Att::WcharTToStr(wchar_t data) const
{
    char buf[5];
    memset(buf, 0, 5);
    sprintf(buf, "%.4X", data);
    return StringFormat("U+%s", buf);
}

// Basic converters for reading

double Att::StrToDbl(std::string value) const
{
    return atof(value.c_str());
}

int Att::StrToInt(std::string value) const
{
    return atoi(value.c_str());
}

wchar_t Att::StrToWcharT(std::string value) const
{
    std::string prefix1 = "U+";
    std::string prefix2 = "#x";
    if (value.compare(0, prefix1.length(), prefix1) == 0) {
        value.erase(0, 2);
    }
    else if (value.compare(0, prefix2.length(), prefix2) == 0) {
        value.erase(0, 2);
    }
    else {
        LogWarning("Unable to parse glyph code '%s'", value.c_str());
        return 0;
    }
    wchar_t wc = (wchar_t)strtol(value.c_str(), NULL, 16);
    // Check that the value is in a SMuFL private area range - this does not check that it is an
    // existing SMuFL glyph num or that it is supported by Verovio
    if ((wc >= 0xE000) && (wc <= 0xF8FF))
        return wc;
    else
        LogWarning("Value '%s' is not in the SMuFL (private area) range", value.c_str());
    return 0;
}

// Converters for writing and reading

std::string Att::ArticulationListToStr(data_ARTICULATION_List data) const
{
    std::ostringstream ss;
    for (size_t i = 0; i < data.size(); ++i) {
        if (i != 0) ss << " ";
        ss << ArticulationToStr(data[i]);
    }
    return ss.str();
}

data_ARTICULATION_List Att::StrToArticulationList(std::string value) const
{
    data_ARTICULATION_List list;
    std::istringstream iss(value);
    std::string token;
    while (std::getline(iss, token, ' ')) {
        list.push_back(StrToArticulation(token));
    }
    return list;
}

std::string Att::BeatrptRendToStr(data_BEATRPT_REND data) const
{
    std::string value;
    switch (data) {
        case BEATRPT_REND_4: value = "4"; break;
        case BEATRPT_REND_8: value = "8"; break;
        case BEATRPT_REND_16: value = "16"; break;
        case BEATRPT_REND_32: value = "32"; break;
        case BEATRPT_REND_64: value = "64"; break;
        case BEATRPT_REND_128: value = "128"; break;
        case BEATRPT_REND_mixed: value = "mixed"; break;
        default:
            LogWarning("Unknown beatrpt rend '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_BEATRPT_REND Att::StrToBeatrptRend(std::string value) const
{
    if (value == "4") return BEATRPT_REND_4;
    if (value == "8") return BEATRPT_REND_8;
    if (value == "16") return BEATRPT_REND_16;
    if (value == "32") return BEATRPT_REND_32;
    if (value == "64") return BEATRPT_REND_64;
    if (value == "128") return BEATRPT_REND_128;
    if (value == "mixed") return BEATRPT_REND_mixed;
    LogWarning("Unsupported beatrpt rend '%s'", value.c_str());
    return BEATRPT_REND_NONE;
}

std::string Att::DurationToStr(data_DURATION data) const
{
    std::string value;
    switch (data) {
        case DURATION_maxima: value = "maxima"; break;
        case DURATION_longa: value = "longa"; break;
        case DURATION_brevis: value = "brevis"; break;
        case DURATION_semibrevis: value = "semibrevis"; break;
        case DURATION_minima: value = "minima"; break;
        case DURATION_semiminima: value = "semiminima"; break;
        case DURATION_fusa: value = "fusa"; break;
        case DURATION_semifusa: value = "semifusa"; break;
        case DURATION_long: value = "long"; break;
        case DURATION_breve: value = "breve"; break;
        case DURATION_1: value = "1"; break;
        case DURATION_2: value = "2"; break;
        case DURATION_4: value = "4"; break;
        case DURATION_8: value = "8"; break;
        case DURATION_16: value = "16"; break;
        case DURATION_32: value = "32"; break;
        case DURATION_64: value = "64"; break;
        case DURATION_128: value = "128"; break;
        case DURATION_256: value = "256"; break;
        default:
            LogWarning("Unknown dur '%d'", data);
            value = "4";
            break;
    }
    return value;
}

data_DURATION Att::StrToDuration(std::string value) const
{
    if (value == "maxima") return DURATION_maxima;
    if (value == "longa") return DURATION_longa;
    if (value == "brevis") return DURATION_brevis;
    if (value == "semibrevis") return DURATION_semibrevis;
    if (value == "minima") return DURATION_minima;
    if (value == "semiminima") return DURATION_semiminima;
    if (value == "fusa") return DURATION_fusa;
    if (value == "semifusa") return DURATION_semifusa;
    if (value == "long") return DURATION_long;
    if (value == "breve") return DURATION_breve;
    if (value == "1") return DURATION_1;
    if (value == "2") return DURATION_2;
    if (value == "4") return DURATION_4;
    if (value == "8") return DURATION_8;
    if (value == "16") return DURATION_16;
    if (value == "32") return DURATION_32;
    if (value == "64") return DURATION_64;
    if (value == "128") return DURATION_128;
    if (value == "256") return DURATION_256;
    if ((value.length() > 0) && (value[value.length() - 1] == 'p')) {
        // LogWarning("PPQ duration dur_s are not supported"); // remove it for now
    }
    else {
        LogWarning("Unknown dur '%s'", value.c_str());
    }
    return DURATION_NONE;
}

std::string Att::PPQToStr(double data) const
{
    std::stringstream sstm;
    sstm << data << "p";

    return sstm.str();;
}

double Att::StrToPPQ(std::string value) const
{
    if ((value.length() > 0) && (value[value.length() - 1] == 'p')) {
        return atof(value.substr(0, value.length() - 1).c_str());
    }
    else {
        LogWarning("Unknown dur ppq '%s'", value.c_str());
    }
    return -1;
}

std::string Att::KeysignatureToStr(data_KEYSIGNATURE data) const
{
    std::string value;
    switch (data) {
        case KEYSIGNATURE_7f: value = "7f"; break;
        case KEYSIGNATURE_6f: value = "6f"; break;
        case KEYSIGNATURE_5f: value = "5f"; break;
        case KEYSIGNATURE_4f: value = "4f"; break;
        case KEYSIGNATURE_3f: value = "3f"; break;
        case KEYSIGNATURE_2f: value = "2f"; break;
        case KEYSIGNATURE_1f: value = "1f"; break;
        case KEYSIGNATURE_0: value = "0"; break;
        case KEYSIGNATURE_1s: value = "1s"; break;
        case KEYSIGNATURE_2s: value = "2s"; break;
        case KEYSIGNATURE_3s: value = "3s"; break;
        case KEYSIGNATURE_4s: value = "4s"; break;
        case KEYSIGNATURE_5s: value = "5s"; break;
        case KEYSIGNATURE_6s: value = "6s"; break;
        case KEYSIGNATURE_7s: value = "7s"; break;
        case KEYSIGNATURE_mixed: value = "mixed"; break;
        default:
            LogWarning("Unknown key signature '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_KEYSIGNATURE Att::StrToKeysignature(std::string value) const
{
    if (value == "7f") return KEYSIGNATURE_7f;
    if (value == "6f") return KEYSIGNATURE_6f;
    if (value == "5f") return KEYSIGNATURE_5f;
    if (value == "4f") return KEYSIGNATURE_4f;
    if (value == "3f") return KEYSIGNATURE_3f;
    if (value == "2f") return KEYSIGNATURE_2f;
    if (value == "1f") return KEYSIGNATURE_1f;
    if (value == "0") return KEYSIGNATURE_0;
    if (value == "1s") return KEYSIGNATURE_1s;
    if (value == "2s") return KEYSIGNATURE_2s;
    if (value == "3s") return KEYSIGNATURE_3s;
    if (value == "4s") return KEYSIGNATURE_4s;
    if (value == "5s") return KEYSIGNATURE_5s;
    if (value == "6s") return KEYSIGNATURE_6s;
    if (value == "7s") return KEYSIGNATURE_7s;
    if (value == "mixed") return KEYSIGNATURE_mixed;
    LogWarning("Unsupported key signature '%s'", value.c_str());
    return KEYSIGNATURE_NONE;
}

std::string Att::MeasurebeatToStr(data_MEASUREBEAT data) const
{
    return StringFormat("%dm+%.1f", data.first, data.second);
}

data_MEASUREBEAT Att::StrToMeasurebeat(std::string value) const
{
    for (size_t i = 0; i < value.length(); i++) {
        if (iswspace(value[i])) {
            value.erase(i, 1);
            i--;
        }
    }
    int measure = 0;
    double timePoint = 0.0;
    int m = (int)value.find_first_of('m');
    int plus = (int)value.find_last_of('+');
    if (m != -1) measure = atoi(value.substr(0, m).c_str());
    if (plus != -1) {
        timePoint = atof(value.substr(plus).c_str());
    }
    else {
        timePoint = atof(value.c_str());
    }
    return std::make_pair(measure, timePoint);
}

std::string Att::ModusmaiorToStr(data_MODUSMAIOR data) const
{
    std::string value;
    switch (data) {
        case MODUSMAIOR_2: value = "2"; break;
        case MODUSMAIOR_3: value = "3"; break;
        default:
            LogWarning("Unknown modusmaior '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_MODUSMAIOR Att::StrToModusmaior(std::string value) const
{
    if (value == "2") return MODUSMAIOR_2;
    if (value == "3") return MODUSMAIOR_3;
    LogWarning("Unsupported modusmaior '%s'", value.c_str());
    return MODUSMAIOR_NONE;
}

std::string Att::ModusminorToStr(data_MODUSMINOR data) const
{
    std::string value;
    switch (data) {
        case MODUSMINOR_2: value = "2"; break;
        case MODUSMINOR_3: value = "3"; break;
        default:
            LogWarning("Unknown modusmaior '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_MODUSMINOR Att::StrToModusminor(std::string value) const
{
    if (value == "2") return MODUSMINOR_2;
    if (value == "3") return MODUSMINOR_3;
    LogWarning("Unsupported modusmaior '%s'", value.c_str());
    return MODUSMINOR_NONE;
}

std::string Att::PercentToStr(data_PERCENT data) const
{
    return StringFormat("%d%%", data);
}

data_PERCENT Att::StrToPercent(std::string value) const
{
    return atoi(value.substr(0, value.find("%")).c_str());
}

std::string Att::PitchnameToStr(data_PITCHNAME data) const
{
    std::string value;
    switch (data) {
        case PITCHNAME_c: value = "c"; break;
        case PITCHNAME_d: value = "d"; break;
        case PITCHNAME_e: value = "e"; break;
        case PITCHNAME_f: value = "f"; break;
        case PITCHNAME_g: value = "g"; break;
        case PITCHNAME_a: value = "a"; break;
        case PITCHNAME_b: value = "b"; break;
        default:
            LogWarning("Unknown pitch name '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_PITCHNAME Att::StrToPitchname(std::string value) const
{
    if (value == "c") return PITCHNAME_c;
    if (value == "d") return PITCHNAME_d;
    if (value == "e") return PITCHNAME_e;
    if (value == "f") return PITCHNAME_f;
    if (value == "g") return PITCHNAME_g;
    if (value == "a") return PITCHNAME_a;
    if (value == "b") return PITCHNAME_b;
    LogWarning("Unsupported pitch name '%s'", value.c_str());
    return PITCHNAME_NONE;
}

std::string Att::OctaveDisToStr(data_OCTAVE_DIS data) const
{
    std::string value;
    switch (data) {
        case OCTAVE_DIS_8: value = "8"; break;
        case OCTAVE_DIS_15: value = "15"; break;
        case OCTAVE_DIS_22: value = "22"; break;
        default:
            LogWarning("Unknown octave dis '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_OCTAVE_DIS Att::StrToOctaveDis(std::string value) const
{
    if (value == "8") return OCTAVE_DIS_8;
    if (value == "15") return OCTAVE_DIS_15;
    if (value == "22") return OCTAVE_DIS_22;
    LogWarning("Unsupported octave dis '%s'", value.c_str());
    return OCTAVE_DIS_NONE;
}

std::string Att::OrientationToStr(data_ORIENTATION data) const
{
    std::string value;
    switch (data) {
        case ORIENTATION_reversed: value = "reversed"; break;
        case ORIENTATION_90CW: value = "90CW"; break;
        case ORIENTATION_90CCW: value = "90CCW"; break;
        default:
            LogWarning("Unknown orientation '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_ORIENTATION Att::StrToOrientation(std::string value) const
{
    if (value == "reversed") return ORIENTATION_reversed;
    if (value == "90CW") return ORIENTATION_90CW;
    if (value == "90CCW") return ORIENTATION_90CCW;
    LogWarning("Unsupported orientation '%s'", value.c_str());
    return ORIENTATION_NONE;
}

std::string Att::ProlatioToStr(data_PROLATIO data) const
{
    std::string value;
    switch (data) {
        case PROLATIO_2: value = "2"; break;
        case PROLATIO_3: value = "3"; break;
        default:
            LogWarning("Unknown prolatio '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_PROLATIO Att::StrToProlatio(std::string value) const
{
    if (value == "2") return PROLATIO_2;
    if (value == "3") return PROLATIO_3;
    LogWarning("Unsupported prolatio '%s'", value.c_str());
    return PROLATIO_NONE;
}

std::string Att::StemdirectionToStr(data_STEMDIRECTION data) const
{
    std::string value;
    switch (data) {
        case STEMDIRECTION_up: value = "up"; break;
        case STEMDIRECTION_down: value = "down"; break;
        default:
            LogWarning("Unknown stem direction '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_STEMDIRECTION Att::StrToStemdirection(std::string value) const
{
    if (value == "up") return STEMDIRECTION_up;
    if (value == "down") return STEMDIRECTION_down;
    LogWarning("Unsupported stem direction '%s'", value.c_str());
    return STEMDIRECTION_NONE;
}

std::string Att::TempusToStr(data_TEMPUS data) const
{
    std::string value;
    switch (data) {
        case TEMPUS_2: value = "2"; break;
        case TEMPUS_3: value = "3"; break;
        default:
            LogWarning("Unknown tempus '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_TEMPUS Att::StrToTempus(std::string value) const
{
    if (value == "2") return TEMPUS_2;
    if (value == "3") return TEMPUS_3;
    LogWarning("Unsupported tempus '%s'", value.c_str());
    return TEMPUS_NONE;
}

std::string Att::TieToStr(data_TIE data) const
{
    std::string value;
    switch (data) {
        case TIE_i: value = "i"; break;
        case TIE_m: value = "m"; break;
        case TIE_t: value = "t"; break;
        default:
            LogWarning("Unknown tie '%d'", data);
            value = "";
            break;
    }
    return value;
}

data_TIE Att::StrToTie(std::string value) const
{
    if (value == "i") return TIE_i;
    if (value == "m") return TIE_m;
    if (value == "t") return TIE_t;
    LogWarning("Unsupported tie '%s'", value.c_str());
    return TIE_NONE;
}

std::string Att::XsdPositiveIntegerListToStr(xsdPositiveInteger_List data) const
{
    std::ostringstream ss;
    for (size_t i = 0; i < data.size(); ++i) {
        if (i != 0) ss << " ";
        ss << data[i];
    }
    return ss.str();
}

xsdPositiveInteger_List Att::StrToXsdPositiveIntegerList(std::string value) const
{
    xsdPositiveInteger_List list;
    std::istringstream iss(value);
    std::string token;
    while (std::getline(iss, token, ' ')) {
        list.push_back(atoi(token.c_str()));
    }
    return list;
}

//----------------------------------------------------------------------------
// AttComparison
//----------------------------------------------------------------------------

bool AttComparison::operator()(Object *object)
{
    if (object->Is(m_classId)) {
        return true;
    }
    return false;
}

bool AttComparison::MatchesType(Object *object)
{
    if (object->Is(m_classId)) {
        return true;
    }
    return false;
}

} // namespace vrv
