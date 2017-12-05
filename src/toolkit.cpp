/////////////////////////////////////////////////////////////////////////////
// Name:        toolkit.cpp
// Author:      Laurent Pugin
// Created:     17/10/2013
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "toolkit.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "attcomparison.h"
#include "attconverter.h"
#include "iodarms.h"
#include "iohumdrum.h"
#include "iomei.h"
#include "iomusxml.h"
#include "iopae.h"
#include "layer.h"
#include "measure.h"
#include "note.h"
#include "page.h"
#include "slur.h"
#include "style.h"
#include "svgdevicecontext.h"
#include "vrv.h"

#include "functorparams.h"

//----------------------------------------------------------------------------

#include "MidiFile.h"

namespace vrv {

const char *UTF_16_BE_BOM = "\xFE\xFF";
const char *UTF_16_LE_BOM = "\xFF\xFE";

//----------------------------------------------------------------------------
// Toolkit
//----------------------------------------------------------------------------

char *Toolkit::m_humdrumBuffer = NULL;

Toolkit::Toolkit(bool initFont)
{
    m_scale = DEFAULT_SCALE;
    m_format = AUTO;

    // default page size
    m_pageHeight = DEFAULT_PAGE_HEIGHT;
    m_pageWidth = DEFAULT_PAGE_WIDTH;
    m_border = DEFAULT_PAGE_LEFT_MAR;
    m_spacingLinear = DEFAULT_SPACING_LINEAR;
    m_spacingNonLinear = DEFAULT_SPACING_NON_LINEAR;
    m_spacingStaff = DEFAULT_SPACING_STAFF;
    m_spacingSystem = DEFAULT_SPACING_SYSTEM;

    m_noLayout = false;
    m_ignoreLayout = false;
    m_adjustPageHeight = false;
    m_noJustification = false;
    m_evenNoteSpacing = false;
    m_showBoundingBoxes = false;
    m_scoreBasedMei = false;
    m_backgroundOpacity = 1.0;

    m_cString = NULL;
    m_humdrumBuffer = NULL;

    if (initFont) {
        Resources::InitFonts();
    }
}

Toolkit::~Toolkit()
{
    if (m_cString) {
        free(m_cString);
        m_cString = NULL;
    }
    if (m_humdrumBuffer) {
        free(m_humdrumBuffer);
        m_humdrumBuffer = NULL;
    }
}

bool Toolkit::SetResourcePath(const std::string &path)
{
    Resources::SetPath(path);
    return Resources::InitFonts();
};

bool Toolkit::SetBackgroundData(const std::string data)
{
    m_backgroundData = data;
    return true;
};

bool Toolkit::SetBackgroundOpacity(float opacity)
{
    if (opacity < 0 || opacity > 1.0) {
        LogError("Opacity out of bounds; default is %d, minimum is %d, and maximum is %d", 1.0,
                 0.0, 1.0);
        return false;
    }

    m_backgroundOpacity = opacity;
    return true;
};

bool Toolkit::SetBorder(int border)
{
    // We use left margin values because for now we cannot specify different values for each margin
    if (border < MIN_PAGE_LEFT_MAR || border > MAX_PAGE_LEFT_MAR) {
        LogError("Border out of bounds; default is %d, minimum is %d, and maximum is %d", DEFAULT_PAGE_LEFT_MAR,
            MIN_PAGE_LEFT_MAR, MAX_PAGE_LEFT_MAR);
        return false;
    }
    m_border = border;
    return true;
}

bool Toolkit::SetScale(int scale)
{
    if (scale < MIN_SCALE || scale > MAX_SCALE) {
        LogError("Scale out of bounds; default is %d, minimum is %d, and maximum is %d", DEFAULT_SCALE, MIN_SCALE,
            MAX_SCALE);
        return false;
    }
    m_scale = scale;
    return true;
}

bool Toolkit::SetPageHeight(int h)
{
    if (h < MIN_PAGE_HEIGHT || h > MAX_PAGE_HEIGHT) {
        LogError("Page height out of bounds; default is %d, minimum is %d, and maximum is %d", DEFAULT_PAGE_HEIGHT,
            MIN_PAGE_HEIGHT, MAX_PAGE_HEIGHT);
        return false;
    }
    m_pageHeight = h;
    return true;
}

bool Toolkit::SetPageWidth(int w)
{
    if (w < MIN_PAGE_WIDTH || w > MAX_PAGE_WIDTH) {
        LogError("Page width out of bounds; default is %d, minimum is %d, and maximum is %d", DEFAULT_PAGE_WIDTH,
            MIN_PAGE_WIDTH, MAX_PAGE_WIDTH);
        return false;
    }
    m_pageWidth = w;
    return true;
};

bool Toolkit::SetSpacingStaff(int spacingStaff)
{
    if (spacingStaff < MIN_SPACING_STAFF || spacingStaff > MAX_SPACING_STAFF) {
        LogError("Spacing staff out of bounds; default is %d, minimum is %d, and maximum is %d", DEFAULT_SPACING_STAFF,
            MIN_SPACING_STAFF, MAX_SPACING_STAFF);
        return false;
    }
    m_spacingStaff = spacingStaff;
    return true;
}

bool Toolkit::SetSpacingSystem(int spacingSystem)
{
    if (spacingSystem < MIN_SPACING_SYSTEM || spacingSystem > MAX_SPACING_SYSTEM) {
        LogError("Spacing system out of bounds; default is %d, minimum is %d, and maximum is %d",
            DEFAULT_SPACING_SYSTEM, MIN_SPACING_SYSTEM, MAX_SPACING_SYSTEM);
        return false;
    }
    m_spacingSystem = spacingSystem;
    return true;
}

bool Toolkit::SetSpacingLinear(float spacingLinear)
{
    if (spacingLinear < MIN_SPACING_LINEAR || spacingLinear > MAX_SPACING_LINEAR) {
        LogError("Spacing (linear) out of bounds; default is %d, minimum is %d, and maximum is %d",
            DEFAULT_SPACING_LINEAR, MIN_SPACING_LINEAR, MAX_SPACING_LINEAR);
        return false;
    }
    m_spacingLinear = spacingLinear;
    return true;
}

bool Toolkit::SetSpacingNonLinear(float spacingNonLinear)
{
    if (spacingNonLinear < MIN_SPACING_NON_LINEAR || spacingNonLinear > MAX_SPACING_NON_LINEAR) {
        LogError("Spacing (non-linear) out of bounds; default is %d, minimum is %d, and maximum is %d",
            DEFAULT_SPACING_NON_LINEAR, MIN_SPACING_NON_LINEAR, MAX_SPACING_NON_LINEAR);
        return false;
    }
    m_spacingNonLinear = spacingNonLinear;
    return true;
}

void Toolkit::SetHeader(std::string title, std::string subtitle, std::string composer, std::string arrangement)
{
    m_doc.m_title = title;
    m_doc.m_subtitle = subtitle;
    m_doc.m_composer = composer;
    m_doc.m_arrangement = arrangement;

    // Have to re-cast off the doc so that there is no system overflow due to the new header
    m_doc.UnCastOffDoc();
    m_doc.CastOffDoc();
}

bool Toolkit::SetOutputFormat(std::string const &outformat)
{
    if (outformat == "humdrum") {
        m_outformat = HUMDRUM;
    }
    else if (outformat == "mei") {
        m_outformat = MEI;
    }
    else if (outformat == "midi") {
        m_outformat = MIDI;
    }
    else if (outformat != "svg") {
        LogError("Output format can only be: mei, humdrum, midi or svg");
        return false;
    }
    return true;
}

bool Toolkit::SetFormat(std::string const &informat)
{
    if (informat == "pae") {
        m_format = PAE;
    }
    else if (informat == "darms") {
        m_format = DARMS;
    }
    else if (informat == "humdrum") {
        m_format = HUMDRUM;
    }
    else if (informat == "mei") {
        m_format = MEI;
    }
    else if (informat == "musicxml") {
        m_format = MUSICXML;
    }
    else if (informat == "musicxml-hum") {
        m_format = MUSICXMLHUM;
    }
    else if (informat == "auto") {
        m_format = AUTO;
    }
    else {
        LogError("Input format can only be: mei, humdrum, pae, musicxml or darms");
        return false;
    }
    return true;
}

void Toolkit::SetAppXPathQueries(std::vector<std::string> const &xPathQueries)
{
    m_appXPathQueries = xPathQueries;
    m_appXPathQueries.erase(std::remove_if(m_appXPathQueries.begin(), m_appXPathQueries.end(),
                                [](const std::string &s) { return s.empty(); }),
        m_appXPathQueries.end());
}

void Toolkit::SetChoiceXPathQueries(std::vector<std::string> const &xPathQueries)
{
    m_choiceXPathQueries = xPathQueries;
    m_choiceXPathQueries.erase(std::remove_if(m_choiceXPathQueries.begin(), m_choiceXPathQueries.end(),
                                   [](const std::string &s) { return s.empty(); }),
        m_choiceXPathQueries.end());
}

FileFormat Toolkit::IdentifyInputFormat(const string &data)
{
#ifdef MUSICXML_DEFAULT_HUMDRUM
    FileFormat musicxmlDefault = MUSICXMLHUM;
#else
    FileFormat musicxmlDefault = MUSICXML;
#endif

    size_t searchLimit = 600;
    if (data.size() == 0) {
        return UNKNOWN;
    }
    if (data[0] == 0) {
        return UNKNOWN;
    }
    if (data[0] == '@') {
        return PAE;
    }
    if (data[0] == '*' || data[0] == '!') {
        return HUMDRUM;
    }
    if ((unsigned int)data[0] == 0xff || (unsigned int)data[0] == 0xfe) {
        // Handle UTF-16 content here later.
        cerr << "Warning: Cannot yet auto-detect format of UTF-16 data files." << endl;
        return UNKNOWN;
    }
    if (data[0] == '<') {
        // <mei> == root node for standard organization of MEI data
        // <pages> == root node for pages organization of MEI data
        // <score-partwise> == root node for part-wise organization of MusicXML data
        // <score-timewise> == root node for time-wise organization of MusicXML data
        // <opus> == root node for multi-movement/work organization of MusicXML data
        string initial = data.substr(0, searchLimit);

        if (initial.find("<mei ") != string::npos) {
            return MEI;
        }
        if (initial.find("<mei>") != string::npos) {
            return MEI;
        }
        if (initial.find("<music>") != string::npos) {
            return MEI;
        }
        if (initial.find("<music ") != string::npos) {
            return MEI;
        }
        if (initial.find("<pages>") != string::npos) {
            return MEI;
        }
        if (initial.find("<pages ") != string::npos) {
            return MEI;
        }
        if (initial.find("<score-partwise>") != string::npos) {
            return musicxmlDefault;
        }
        if (initial.find("<score-timewise>") != string::npos) {
            return musicxmlDefault;
        }
        if (initial.find("<opus>") != string::npos) {
            return musicxmlDefault;
        }
        if (initial.find("<score-partwise ") != string::npos) {
            return musicxmlDefault;
        }
        if (initial.find("<score-timewise ") != string::npos) {
            return musicxmlDefault;
        }
        if (initial.find("<opus ") != string::npos) {
            return musicxmlDefault;
        }

        cerr << "Warning: Trying to load unknown XML data which cannot be identified." << endl;
        return UNKNOWN;
    }

    // Assume that the input is MEI if other input types were not detected.
    // This means that DARMS cannot be auto detected.
    return MEI;
}

bool Toolkit::SetFont(std::string const &font)
{
    return Resources::SetFont(font);
};

bool Toolkit::LoadFile(const std::string &filename)
{
    if (IsUTF16(filename)) {
        return LoadUTF16File(filename);
    }

    std::ifstream in(filename.c_str());
    if (!in.is_open()) {
        return false;
    }

    in.seekg(0, std::ios::end);
    std::streamsize fileSize = (std::streamsize)in.tellg();
    in.clear();
    in.seekg(0, std::ios::beg);

    // read the file into the string:
    std::string content(fileSize, 0);
    in.read(&content[0], fileSize);

    return LoadData(content);
}

bool Toolkit::IsUTF16(const std::string &filename)
{
    std::ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        return false;
    }

    char data[2];
    memset(data, 0, 2);
    fin.read(data, 2);
    fin.close();

    if (memcmp(data, UTF_16_LE_BOM, 2) == 0) return true;
    if (memcmp(data, UTF_16_BE_BOM, 2) == 0) return true;

    return false;
}

bool Toolkit::LoadUTF16File(const std::string &filename)
{
    /// Loading a UTF-16 file with basic conversion ot UTF-8
    /// This is called after checking if the file has a UTF-16 BOM

    LogWarning("The file seems to be UTF-16 - trying to convert to UTF-8");

    std::ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        return false;
    }

    fin.seekg(0, std::ios::end);
    std::streamsize wfileSize = (std::streamsize)fin.tellg();
    fin.clear();
    fin.seekg(0, std::wios::beg);

    std::vector<unsigned short> utf16line;
    utf16line.reserve(wfileSize / 2 + 1);

    unsigned short buffer;
    while (fin.read((char *)&buffer, sizeof(unsigned short))) {
        utf16line.push_back(buffer);
    }
    // LogDebug("%d %d", wfileSize, utf8line.size());

    std::string utf8line;
    utf8::utf16to8(utf16line.begin(), utf16line.end(), back_inserter(utf8line));

    return LoadData(utf8line);
}

bool Toolkit::LoadData(const std::string &data)
{
    string newData;
    FileInputStream *input = NULL;

    auto inputFormat = m_format;
    if (inputFormat == AUTO) {
        inputFormat = IdentifyInputFormat(data);
    }

    if (inputFormat == PAE) {
        input = new PaeInput(&m_doc, "");
    }
    else if (inputFormat == DARMS) {
        input = new DarmsInput(&m_doc, "");
    }
#ifndef NO_HUMDRUM_SUPPORT
    else if (inputFormat == HUMDRUM) {
        // LogMessage("Importing Humdrum data");

        Doc tempdoc;
        HumdrumInput *tempinput = new HumdrumInput(&tempdoc, "");
        tempinput->SetTypeOption(GetHumType());

        if (GetOutputFormat() == HUMDRUM) {
            tempinput->SetOutputFormat("humdrum");
        }

        if (!tempinput->ImportString(data)) {
            LogError("Error importing Humdrum data");
            delete tempinput;
            return false;
        }

        SetHumdrumBuffer(tempinput->GetHumdrumString().c_str());

        if (GetOutputFormat() == HUMDRUM) {
            return true;
        }

        MeiOutput meioutput(&tempdoc, "");
        meioutput.SetScoreBasedMEI(true);
        newData = meioutput.GetOutput();
        delete tempinput;

        input = new MeiInput(&m_doc, "");
    }
#endif
    else if (inputFormat == MEI) {
        input = new MeiInput(&m_doc, "");
    }
    else if (inputFormat == MUSICXML) {
        // This is the direct converter from MusicXML to MEI using iomusicxml:
        input = new MusicXmlInput(&m_doc, "");
    }
#ifndef NO_HUMDRUM_SUPPORT
    else if (inputFormat == MUSICXMLHUM) {
        // This is the indirect converter from MusicXML to MEI using iohumdrum:
        hum::Tool_musicxml2hum converter;
        pugi::xml_document xmlfile;
        xmlfile.load(data.c_str());
        stringstream conversion;
        bool status = converter.convert(conversion, xmlfile);
        if (!status) {
            LogError("Error converting MusicXML");
            return false;
        }
        std::string buffer = conversion.str();
        SetHumdrumBuffer(buffer.c_str());

        // Now convert Humdrum into MEI:
        Doc tempdoc;
        FileInputStream *tempinput = new HumdrumInput(&tempdoc, "");
        tempinput->SetTypeOption(GetHumType());
        if (!tempinput->ImportString(conversion.str())) {
            LogError("Error importing Humdrum data");
            delete tempinput;
            return false;
        }
        MeiOutput meioutput(&tempdoc, "");
        meioutput.SetScoreBasedMEI(true);
        newData = meioutput.GetOutput();
        delete tempinput;
        input = new MeiInput(&m_doc, "");
    }
#endif
    else {
        LogMessage("Unsupported format");
        return false;
    }

    // something went wrong
    if (!input) {
        LogError("Unknown error");
        return false;
    }

    // xpath queries?
    if (m_appXPathQueries.size() > 0) {
        input->SetAppXPathQueries(m_appXPathQueries);
    }
    if (m_choiceXPathQueries.size() > 0) {
        input->SetChoiceXPathQueries(m_choiceXPathQueries);
    }
    if (m_mdivXPathQuery.length() > 0) {
        input->SetMdivXPathQuery(m_mdivXPathQuery);
    }

    // load the file
    if (!input->ImportString(newData.size() ? newData : data)) {
        LogError("Error importing data");
        delete input;
        return false;
    }

    m_doc.SetPageHeight(this->GetPageHeight());
    m_doc.SetPageWidth(this->GetPageWidth());
    m_doc.SetPageRightMar(this->GetBorder());
    m_doc.SetPageLeftMar(this->GetBorder());
    m_doc.SetPageTopMar(this->GetBorder());
    m_doc.SetSpacingLinear(this->GetSpacingLinear());
    m_doc.SetSpacingNonLinear(this->GetSpacingNonLinear());
    m_doc.SetSpacingStaff(this->GetSpacingStaff());
    m_doc.SetSpacingSystem(this->GetSpacingSystem());
    m_doc.SetEvenSpacing(this->GetEvenNoteSpacing());

    m_doc.PrepareDrawing();

    // Do the layout? this depends on the options and the file. PAE and
    // DARMS have no layout information. MEI files _can_ have it, but it
    // might have been ignored because of the --ignore-layout option.
    // Regardless, we won't do layout if the --no-layout option was set.
    if (!m_noLayout) {
        if (input->HasLayoutInformation() && !m_ignoreLayout) {
            // LogElapsedTimeStart();
            m_doc.CastOffEncodingDoc();
            // LogElapsedTimeEnd("layout");
        }
        else {
            // LogElapsedTimeStart();
            m_doc.CastOffDoc();
            // LogElapsedTimeEnd("layout");
        }
    }

    // disable justification if there's no layout or no justification
    if (m_noLayout || m_noJustification) {
        m_doc.SetJustificationX(false);
    }

    delete input;
    m_view.SetDoc(&m_doc);
    m_transpose = new Transpose(&m_doc);

    return true;
}

std::string Toolkit::GetMEI(int pageNo, bool scoreBased)
{
    m_doc.m_header.reset();
    // Page number is one-based - correct it to 0-based first
    pageNo--;

    MeiOutput meioutput(&m_doc, "");
    meioutput.SetScoreBasedMEI(scoreBased);
    return meioutput.GetOutput(pageNo);
}

bool Toolkit::SaveFile(const std::string &filename)
{
    MeiOutput meioutput(&m_doc, filename.c_str());
    meioutput.SetScoreBasedMEI(m_scoreBasedMei);
    if (!meioutput.ExportFile()) {
        LogError("Unknown error");
        return false;
    }
    return true;
}

bool Toolkit::ParseOptions(const std::string &json_options)
{
#if defined(USE_EMSCRIPTEN) || defined(PYTHON_BINDING)

    jsonxx::Object json;

    // Read JSON options
    if (!json.parse(json_options)) {
        LogError("Can not parse JSON string.");
        return false;
    }

    if (json.has<jsonxx::String>("inputFormat")) SetFormat(json.get<jsonxx::String>("inputFormat"));

    if (json.has<jsonxx::Number>("scale")) SetScale(json.get<jsonxx::Number>("scale"));

	if (json.has<jsonxx::Number>("backgroundOpacity")) SetBackgroundOpacity(json.get<jsonxx::Number>("backgroundOpacity"));

	if (json.has<jsonxx::String>("backgroundData")) SetBackgroundData(json.get<jsonxx::String>("backgroundData"));

    if (json.has<jsonxx::Number>("border")) SetBorder(json.get<jsonxx::Number>("border"));

    if (json.has<jsonxx::String>("font")) SetFont(json.get<jsonxx::String>("font"));

    if (json.has<jsonxx::Number>("pageWidth")) SetPageWidth(json.get<jsonxx::Number>("pageWidth"));

    if (json.has<jsonxx::Number>("pageHeight")) SetPageHeight(json.get<jsonxx::Number>("pageHeight"));

    if (json.has<jsonxx::Number>("spacingLinear")) SetSpacingLinear(json.get<jsonxx::Number>("spacingLinear"));

    if (json.has<jsonxx::Number>("spacingNonLinear")) SetSpacingNonLinear(json.get<jsonxx::Number>("spacingNonLinear"));

    if (json.has<jsonxx::Number>("spacingStaff")) SetSpacingStaff(json.get<jsonxx::Number>("spacingStaff"));

    if (json.has<jsonxx::Number>("spacingSystem")) SetSpacingSystem(json.get<jsonxx::Number>("spacingSystem"));

	if (json.has<jsonxx::Object>("header")) ParseHeader(json.get<jsonxx::Object>("header"));

    if (json.has<jsonxx::String>("appXPathQuery")) {
        std::vector<std::string> queries = { json.get<jsonxx::String>("appXPathQuery") };
        SetAppXPathQueries(queries);
    }

    if (json.has<jsonxx::Array>("appXPathQueries")) {
        jsonxx::Array values = json.get<jsonxx::Array>("appXPathQueries");
        std::vector<std::string> queries;
        int i;
        for (i = 0; i < values.size(); i++) {
            if (values.has<jsonxx::String>(i)) queries.push_back(values.get<jsonxx::String>(i));
        }
        SetAppXPathQueries(queries);
    }

    if (json.has<jsonxx::String>("choiceXPathQuery")) {
        std::vector<std::string> queries = { json.get<jsonxx::String>("choiceXPathQuery") };
        SetChoiceXPathQueries(queries);
    }

    if (json.has<jsonxx::Array>("choiceXPathQueries")) {
        jsonxx::Array values = json.get<jsonxx::Array>("choiceXPathQueries");
        std::vector<std::string> queries;
        int i;
        for (i = 0; i < values.size(); i++) {
            if (values.has<jsonxx::String>(i)) queries.push_back(values.get<jsonxx::String>(i));
        }
        SetChoiceXPathQueries(queries);
    }

    if (json.has<jsonxx::String>("mdivXPathQuery")) SetMdivXPathQuery(json.get<jsonxx::String>("mdivXPathQuery"));

    if (json.has<jsonxx::Number>("xmlIdSeed")) Object::SeedUuid(json.get<jsonxx::Number>("xmlIdSeed"));

    // Parse the various flags
    // Note: it seems that there is a bug with jsonxx and emscripten
    // Boolean value false won't be parsed properly. We have to use Number instead

    if (json.has<jsonxx::Number>("noLayout")) SetNoLayout(json.get<jsonxx::Number>("noLayout"));

    if (json.has<jsonxx::Number>("ignoreLayout")) SetIgnoreLayout(json.get<jsonxx::Number>("ignoreLayout"));

    if (json.has<jsonxx::Number>("adjustPageHeight")) SetAdjustPageHeight(json.get<jsonxx::Number>("adjustPageHeight"));

    if (json.has<jsonxx::Number>("noJustification")) SetNoJustification(json.get<jsonxx::Number>("noJustification"));

    if (json.has<jsonxx::Number>("humType")) {
        SetHumType(json.get<jsonxx::Number>("humType"));
    }

    if (json.has<jsonxx::Number>("showBoundingBoxes"))
        SetShowBoundingBoxes(json.get<jsonxx::Number>("showBoundingBoxes"));

    return true;
#else
    // The non-js version of the app should not use this function.
    return false;
#endif
}

std::string Toolkit::GetElementAttr(const std::string &xmlId)
{
#if defined(USE_EMSCRIPTEN) || defined(PYTHON_BINDING)
    jsonxx::Object o;

    if (!m_doc.GetDrawingPage()) return o.json();
    Object *element = m_doc.GetDrawingPage()->FindChildByUuid(xmlId);
    if (!element) {
        LogMessage("Element with id '%s' could not be found", xmlId.c_str());
        return o.json();
    }

    // Fill the attribute array (pair of string) by looking at attributes for all available MEI modules
    ArrayOfStrAttr attributes;
    element->GetAttributes(&attributes);

    // Fill the JSON object
    ArrayOfStrAttr::iterator iter;
    for (iter = attributes.begin(); iter != attributes.end(); iter++) {
        o << (*iter).first << (*iter).second;
        // LogMessage("Element %s - %s", (*iter).first.c_str(), (*iter).second.c_str());
    }
    return o.json();

#else
    // The non-js version of the app should not use this function.
    return "";
#endif
}


#ifdef USE_EMSCRIPTEN
void Toolkit::ParseHeader(jsonxx::Object header)
{
	std::string title, subtitle, composer, arrangement;

	if (header.has<jsonxx::String>("title"))
		title = header.get<jsonxx::String>("title");
	if (header.has<jsonxx::String>("subtitle"))
		subtitle = header.get<jsonxx::String>("subtitle");
	if (header.has<jsonxx::String>("composer"))
		composer = header.get<jsonxx::String>("composer");
	if (header.has<jsonxx::String>("arrangement"))
		arrangement = header.get<jsonxx::String>("arrangement");

	SetHeader(title, subtitle, composer, arrangement);
}
#endif

bool Toolkit::Edit(const std::string &json_editorAction)
{
#ifdef USE_EMSCRIPTEN

    jsonxx::Object json;

    // Read JSON actions
    if (!json.parse(json_editorAction)) {
        LogError("Can not parse JSON string.");
        return false;
    }

    if (json.has<jsonxx::String>("action") && json.has<jsonxx::Object>("param")) {
        if (json.get<jsonxx::String>("action") == "drag") {
            std::string elementId;
            int x, y;
            if (this->ParseDragAction(json.get<jsonxx::Object>("param"), &elementId, &x, &y)) {
                return this->Drag(elementId, x, y);
            }
        }
        else if (json.get<jsonxx::String>("action") == "insert") {
            LogMessage("insert...");
            std::string elementType, startid, endid;
            if (this->ParseInsertAction(json.get<jsonxx::Object>("param"), &elementType, &startid, &endid)) {
                return this->Insert(elementType, startid, endid);
            }
            else {
                LogMessage("Insert!!!! %s %s %s", elementType.c_str(), startid.c_str(), endid.c_str());
            }
        }
        else if (json.get<jsonxx::String>("action") == "set") {
            std::string elementId, attrType, attrValue;
            if (this->ParseSetAction(json.get<jsonxx::Object>("param"), &elementId, &attrType, &attrValue)) {
                return this->Set(elementId, attrType, attrValue);
            }
        }
        else if (json.get<jsonxx::String>("action") == "transposeToKey") {
            int newFifths;
            if (this->ParseTransposeKeyAction(json.get<jsonxx::Object>("param"), &newFifths)) {
                return this->TransposeKey(newFifths);
            }
        }
    }
    LogError("Does not understand action.");
    return false;

#else
    // The non-js version of the app should not use this function.
    return false;
#endif
}

std::string Toolkit::GetLogString()
{
#ifdef USE_EMSCRIPTEN
    std::string str;
    std::vector<std::string>::iterator iter;
    for (iter = logBuffer.begin(); iter != logBuffer.end(); iter++) {
        str += (*iter);
    }
    return str;
#else
    // The non-js version of the app should not use this function.
    return "";
#endif
}

std::string Toolkit::GetVersion()
{
    return vrv::GetVersion();
}

void Toolkit::ResetLogBuffer()
{
#ifdef USE_EMSCRIPTEN
    vrv::logBuffer.clear();
#endif
}

void Toolkit::RedoLayout()
{
    if (m_doc.GetType() == Transcription) {
        return;
    }

    m_doc.SetPageHeight(this->GetPageHeight());
    m_doc.SetPageWidth(this->GetPageWidth());
    m_doc.SetPageRightMar(this->GetBorder());
    m_doc.SetPageLeftMar(this->GetBorder());
    m_doc.SetPageTopMar(this->GetBorder());
    m_doc.SetSpacingStaff(this->GetSpacingStaff());
    m_doc.SetSpacingSystem(this->GetSpacingSystem());

    m_doc.UnCastOffDoc();
    m_doc.CastOffDoc();
}

void Toolkit::RedoPagePitchPosLayout()
{
    Page *page = m_doc.GetDrawingPage();

    if (!page) {
        LogError("No page to re-layout");
        return;
    }

    page->LayOutPitchPos();
}

std::string Toolkit::RenderToSvg(int pageNo, bool xml_declaration)
{
    // Page number is one-based - correct it to 0-based first
    pageNo--;

    // Get the current system for the SVG clipping size
    m_view.SetPage(pageNo);

    // Adjusting page width and height according to the options
    int width = m_pageWidth;
    int height = m_pageHeight;

    if (m_noLayout) width = m_doc.GetAdjustedDrawingPageWidth();
    if (m_adjustPageHeight || m_noLayout) height = m_doc.GetAdjustedDrawingPageHeight();

    // Create the SVG object, h & w come from the system
    // We will need to set the size of the page after having drawn it depending on the options
    SvgDeviceContext svg(width, height);

    // set scale and border from user options
    svg.SetUserScale(m_view.GetPPUFactor() * (double)m_scale / 100, m_view.GetPPUFactor() * (double)m_scale / 100);

    // debug BB?
    svg.SetDrawBoundingBoxes(m_showBoundingBoxes);

    svg.SetBackgroundImage(m_backgroundData, m_backgroundOpacity);

    // render the page
    m_view.DrawCurrentPage(&svg, false);

    std::string out_str = svg.GetStringSVG(xml_declaration);
    return out_str;
}

bool Toolkit::RenderToSvgFile(const std::string &filename, int pageNo)
{
    std::string output = RenderToSvg(pageNo, true);

    std::ofstream outfile;
    outfile.open(filename.c_str());

    if (!outfile.is_open()) {
        // add message?
        return false;
    }

    outfile << output;
    outfile.close();
    return true;
}

std::string Toolkit::GetHumdrum()
{
    return GetHumdrumBuffer();
}

bool Toolkit::GetHumdrumFile(const std::string &filename)
{
    std::ofstream output;
    output.open(filename.c_str());

    if (!output.is_open()) {
        // add message?
        return false;
    }

    GetHumdrum(output);
    output.close();
    return true;
}

void Toolkit::GetHumdrum(ostream &output)
{
    output << GetHumdrumBuffer();
}

std::string Toolkit::RenderToMidi()
{
    MidiFile outputfile;
    outputfile.absoluteTicks();
    m_doc.ExportMIDI(&outputfile);
    outputfile.sortTracks();

    stringstream strstrem;
    outputfile.write(strstrem);
    std::string outputstr = Base64Encode(
        reinterpret_cast<const unsigned char *>(strstrem.str().c_str()), (unsigned int)strstrem.str().length());

    return outputstr;
}

std::string Toolkit::GetElementsAtTime(int millisec)
{
#if defined(USE_EMSCRIPTEN) || defined(PYTHON_BINDING)
    jsonxx::Object o;
    jsonxx::Array a;

    double time = (double)(millisec * 120 / 1000);
    NoteOnsetOffsetComparison matchTime(time);
    ArrayOfObjects notes;
    // Here we would need to check that the midi export is done
    if (m_doc.GetMidiExportDone()) {
        m_doc.FindAllChildByAttComparison(&notes, &matchTime);

        // Get the pageNo from the first note (if any)
        int pageNo = -1;
        if (notes.size() > 0) {
            Page *page = dynamic_cast<Page *>(notes.at(0)->GetFirstParent(PAGE));
            if (page) pageNo = page->GetIdx() + 1;
        }

        // Fill the JSON object
        ArrayOfObjects::iterator iter;
        for (iter = notes.begin(); iter != notes.end(); iter++) {
            a << (*iter)->GetUuid();
        }
        o << "notes" << a;
        o << "page" << pageNo;
    }
    return o.json();
#else
    // The non-js version of the app should not use this function.
    return "";
#endif
}

std::string Toolkit::GetKeySignature()
{
#if defined(USE_EMSCRIPTEN) || defined(PYTHON_BINDING)
    jsonxx::Object o;

	o << "fifths" << Transpose::GetFirstKeySigFifths(&m_doc);

	return o.json();
#else
    // The non-js version of the app should not use this function.
    return "";
#endif
}

std::string Toolkit::GetInstruments()
{
#if defined(USE_EMSCRIPTEN) || defined(PYTHON_BINDING)
	ScoreDef scoreDef = m_doc.m_scoreDef;
	StaffGrp *topStaffGrp = dynamic_cast<StaffGrp *>(scoreDef.FindChildByType(STAFFGRP));

        jsonxx::Object output = GetStaffGrp(topStaffGrp);
	return output.json();
#else
    // The non-js version of the app should not use this function.
    return "";
#endif
}

#if defined(USE_EMSCRIPTEN) || defined(PYTHON_BINDING)
jsonxx::Object Toolkit::GetStaffGrp(StaffGrp *staffGrp)
{
	jsonxx::Object output;

        output << "type" << "staffGrp";
	if (staffGrp->HasMidiInstrname())
	{
		output << "midi.instrname" << staffGrp->GetMidiInstrname();
	}
	if (staffGrp->HasLabel())
	{
		output << "label" << staffGrp->GetLabel();
	}
	output << "id" << staffGrp->GetUuid();

        jsonxx::Array children;
	int i;
        for (i = 0; i < staffGrp->GetChildCount(); i++) {
            Object *child = staffGrp->GetChild(i);
            if (child->Is(STAFFGRP)) {
                children << GetStaffGrp(dynamic_cast<StaffGrp *>(child));
            }
            else if (child->Is(STAFFDEF)) {
                children << GetStaffDef(dynamic_cast<StaffDef *>(child));
            }
        }
        output << "children" << children;
	return output;
}

jsonxx::Object Toolkit::GetStaffDef(StaffDef *staffDef)
{
	jsonxx::Object output;

        output << "type" << "staffDef";
	if (staffDef->HasMidiInstrname())
	{
		output << "midi.instrname" << staffDef->GetMidiInstrname();
	}
	if (staffDef->HasLabel())
	{
		output << "label" << staffDef->GetLabel();
	}
	output << "id" << staffDef->GetUuid();

	return output;
}
#endif

bool Toolkit::ChangeInstrument(std::string elementId, std::string json_newInstrument)
{
    // The non-js version of the app should not use this function.
#if defined(USE_EMSCRIPTEN) || defined(PYTHON_BINDING)
    jsonxx::Object json;

    // Read JSON options
    if (!json.parse(json_newInstrument)) {
        LogError("Can not parse JSON string.");
        return false;
    }

    // changingPartObj is either a StaffGrp or StaffDef that the user passes in
    Object *changingPartObj = m_doc.m_scoreDef.FindChildByUuid(elementId);
    if (!changingPartObj) {
        LogError("Cannot find element id!");
        return false;
    }
    if (!changingPartObj->Is(STAFFDEF) && !changingPartObj->Is(STAFFGRP)) {
        LogError("Element id is not a staffgrp or staffdef.");
        return false;
    }

    if (json.has<jsonxx::Object>("partDefaults")) {
        jsonxx::Object partDefaults = json.get<jsonxx::Object>("partDefaults");

        // first, parse the staff element and store the staff data in the newStaffDefs array - includes clefs and TAB 
        std::vector<StaffDef> newStaffDefs;
        if (partDefaults.has<jsonxx::Object>("staff")) {
            jsonxx::Object staffJson = partDefaults.get<jsonxx::Object>("staff");

            std::vector<jsonxx::Object> newStavesJson;
            if (staffJson.has<jsonxx::Object>("clef"))
            {
                jsonxx::Object clef = staffJson.get<jsonxx::Object>("clef");
                newStavesJson.push_back(clef);
            }
            else if (staffJson.has<jsonxx::Array>("clef"))
            {
                jsonxx::Array clefArray = staffJson.get<jsonxx::Array>("clef");
                int i;
                for (i = 0; i < clefArray.size(); i++) {
                    if (clefArray.has<jsonxx::Object>(i)) newStavesJson.push_back(clefArray.get<jsonxx::Object>(i));
                }
            }

            // Go through each clef element on the staff element
            std::vector<jsonxx::Object>::iterator staffObjectIter;
            for (staffObjectIter = newStavesJson.begin(); staffObjectIter != newStavesJson.end(); staffObjectIter++) {
                jsonxx::Object clefJson = *staffObjectIter;
                StaffDef staffDef;
                if (clefJson.has<jsonxx::String>("sign"))
                {
                    std::string clefSignStr = clefJson.get<jsonxx::String>("sign");
                    // cast to a random attribute so that we can call the StrToClefshape method; doesn't matter which attribute
                    AttCommonPart *att = dynamic_cast<AttCommonPart *>(&staffDef);
                    data_CLEFSHAPE clefSign = att->StrToClefshape(clefSignStr);
                    // set the clefshape to the new clef
                    staffDef.SetClefShape(clefSign);

                    // Set the defaults for each clef sign if not provided
                    switch (clefSign)
                    {
                    case CLEFSHAPE_G:
                        staffDef.SetClefLine(2);
                        break;
                    case CLEFSHAPE_GG:
                        staffDef.SetClefLine(2);
                        staffDef.SetClefDis(OCTAVE_DIS_8);
                        staffDef.SetClefDisPlace(PLACE_below);
                        break;
                    case CLEFSHAPE_F:
                        staffDef.SetClefLine(4);
                        break;
                    case CLEFSHAPE_C:
                        break;
                    case CLEFSHAPE_perc:
                        staffDef.SetClefLine(3);
                        break;
                    case CLEFSHAPE_TAB:
                        staffDef.SetClefLine(3);
                        break;
                    case CLEFSHAPE_NONE:
                    default:
                        break;
                    }
                }
                // how many lines are actually displayed
                if (clefJson.has<jsonxx::Number>("displayLineCount"))
                {
                    int displayLineCount = clefJson.get<jsonxx::Number>("displayLineCount");
                    staffDef.SetLines(displayLineCount);
                }
                // which line the clef is on, ex. 2 for treble, 4 for bass
                if (clefJson.has<jsonxx::Number>("line"))
                {
                    int line = clefJson.get<jsonxx::Number>("line");
                    // add one to what is in the manifest
                    staffDef.SetClefLine(line + 1);
                }
                // currently, verovio doens't really support tab, so I'll have to take out tab instruments
                if (staffJson.has<jsonxx::Object>("staffTuning"))
                {
                    jsonxx::Object staffTuning = staffJson.get<jsonxx::Object>("staffTuning");
                    if (staffTuning.has<jsonxx::Array>("note"))
                    {
                        jsonxx::Array staffNoteArray = staffTuning.get<jsonxx::Array>("note");
                        int i;
                        for (i = 0; i < staffNoteArray.size(); i++) {
                            if (staffNoteArray.has<jsonxx::Number>(i)) {
                                int midiNote = staffNoteArray.get<jsonxx::Number>(i);
                                // TODO: TAB NOT IMPLEMENTED
                            }
                        }
                    }
                }
                newStaffDefs.push_back(staffDef);
            }
        }

        // Now, iterate through the existing staffDef elements
        std::vector<StaffDef *> staffDefs;
        if (changingPartObj->Is(STAFFDEF))
        {
            staffDefs.push_back(dynamic_cast<StaffDef *>(changingPartObj));
        }
        // if it's a staffgrp, get all the staffdefs inside of it
        else if (changingPartObj->Is(STAFFGRP))
        {
            ArrayOfObjects staffDefObjs = changingPartObj->FindAllChildByType(STAFFDEF);
            staffDefs.reserve(staffDefObjs.size());

            ArrayOfObjects::iterator iter;
            for (iter = staffDefObjs.begin(); iter != staffDefObjs.end(); iter++) {
                staffDefs.push_back(dynamic_cast<StaffDef *>(*iter));
            }
        }

        // iterate through the existing staffs and set new properties - transpose, clef, etc
        std::vector<StaffDef *>::iterator iter;
        for (iter = staffDefs.begin(); iter != staffDefs.end(); iter++) {
            StaffDef *staffDef = *iter;

            // First, we need to negate any existing transposition to bring the music back to concert pitch
            Transpose::Interval *transpositionInterval = new Transpose::Interval();
            if (staffDef->HasTransDiat() && staffDef->HasTransSemi())
            {
                transpositionInterval = new Transpose::Interval(staffDef->GetTransDiat(), staffDef->GetTransSemi());
            }
            else if (staffDef->HasTransSemi())
            {
                transpositionInterval = Transpose::Interval::FromPitches(staffDef->GetTransSemi());
            }
            // and then reset the part transposition
            staffDef->ResetTransposition();

            // TRANSPOSE
            int transp = 0;
            Transpose::Interval *partTranspose;
            if (partDefaults.has<jsonxx::Number>("transp")) {
                transp = partDefaults.get<jsonxx::Number>("transp");
                // from nf
                partTranspose = Transpose::Interval::FromPitches(-transp)->NormalizeTritone();
                // set the part transposition so that the MIDI file will negate it and play it in concert pitch
                staffDef->SetTransDiat(-partTranspose->GetDiatonic());
                staffDef->SetTransSemi(-partTranspose->GetChromatic());
            }

            // UPDATE STAFFDEF
            StaffDef newStaffDef;

            int index = std::distance(staffDefs.begin(), iter);
            // If there's a new staff to match the old staff, then get the new staff
            if (index < newStaffDefs.size())
            {
                newStaffDef = newStaffDefs[index];
            }
            // Else if you're going from, ex, 2 staves to 1, then just get the last new staff
            else if (!newStaffDefs.empty())
            {
                newStaffDef = newStaffDefs.back();
            }
            // This should never happen because every instrument should have at least one staff
            else
            {
                LogWarning("No new clefs!");
                return false;
            }

            StaffDef *updatedStaffDef = m_doc.m_scoreDef.GetStaffDef(staffDef->GetN());
            // reset the staffdef clef to begin with so that no values are left over
            updatedStaffDef->ResetCleffingLog();
            // update the old clef to the new clef
            if (newStaffDef.HasClefShape())
            {
                updatedStaffDef->SetClefShape(newStaffDef.GetClefShape());
            }
            if (newStaffDef.HasClefLine())
            {
                updatedStaffDef->SetClefLine(newStaffDef.GetClefLine());
            }
            if (newStaffDef.HasClefDis())
            {
                updatedStaffDef->SetClefDis(newStaffDef.GetClefDis());
            }
            if (newStaffDef.HasClefDisPlace())
            {
                updatedStaffDef->SetClefDisPlace(newStaffDef.GetClefDisPlace());
            }
            if (newStaffDef.HasLines())
            {
                updatedStaffDef->SetLines(newStaffDef.GetLines());
            }

            // Automatically transpose the old notes to be within the comf or pro range of the new part 
            if (partDefaults.has<jsonxx::Number>("comfHigh") && partDefaults.has<jsonxx::Number>("comfLow") &&
                partDefaults.has<jsonxx::Number>("proHigh") && partDefaults.has<jsonxx::Number>("proLow")) 
            {
                int comfHigh = partDefaults.get<jsonxx::Number>("comfHigh") - 12;
                int comfLow = partDefaults.get<jsonxx::Number>("comfLow") - 12;
                int proHigh = partDefaults.get<jsonxx::Number>("proHigh") - 12;
                int proLow = partDefaults.get<jsonxx::Number>("proLow") - 12;

                // If there's more than one new and old staff, then we consider it a multiStaff
                // because if you're going from 1 to 2, then I have no clue how that's gonna work
                // and if you're going from 2 to 1, then you want it to be transposed like one part
                bool multiStaff = staffDefs.size() > 1 && newStaffDefs.size() > 1;
                int octaveTransposition = m_transpose->GetPartTransposition(*transpositionInterval, *staffDef, comfHigh, comfLow, proHigh, proLow, multiStaff);
                octaveTransposition /= 12; // get the number of octaves from the chromatic interval
                transpositionInterval = new Transpose::Interval(transpositionInterval->GetDiatonic() + (octaveTransposition * 7), transpositionInterval->GetChromatic() + (octaveTransposition * 12));
            }

            if (partDefaults.has<jsonxx::Number>("oct")) {
                int oct = partDefaults.get<jsonxx::Number>("oct");
                // have to reverse it, idk why but it works that way
                oct = -oct;
                // from nf
                transpositionInterval = new Transpose::Interval(transpositionInterval->GetDiatonic() + (oct * 7), transpositionInterval->GetChromatic() + (oct * 12));
                // Have to set this in it's own property, NOT on transSemi, because we don't want the octave to be negated because then it'll be too far in the opposite direction
                staffDef->SetTransOct(-oct);
            }

            // Combine the part 'transp' and the 'oct'/auto-transpose intervals
            transpositionInterval = new Transpose::Interval(transpositionInterval->GetDiatonic() + partTranspose->GetDiatonic(),
                transpositionInterval->GetChromatic() + partTranspose->GetChromatic());

            // actually do the transposition now that we have everything in one place
            m_transpose->transposeInterval(*transpositionInterval, *staffDef);
        }

        // List of staffdefs or staffgrps we're going to assign name, midi, etc too. Not necessarily the same one we started with, because...
        std::vector<Object *> changingPartObjs;
        // If tehre's more old staves than new staves, then that means we have to break up some staffGrp's, because we don't want them to be tied together forever
        if (staffDefs.size() > newStaffDefs.size())
        {
            StaffDef *firstStaffDef = staffDefs.front();
            Object *parent = firstStaffDef->GetParent();

            // Main StaffGrp
            Object *mainStaffGrp = parent->GetParent();

            if (parent->Is(STAFFGRP) && parent != mainStaffGrp)
            {
                for (int i = newStaffDefs.size() - 1; i != staffDefs.size(); ++i)
                {
                    StaffDef *staffDef = staffDefs[i];
                    StaffDef *updatedStaffDef = m_doc.m_scoreDef.GetStaffDef(staffDef->GetN());
                    // not sure if all this is necessary or best
                    Object *updatedParent = updatedStaffDef->GetParent();

                    if (updatedParent->Is(STAFFGRP))
                    {
                        StaffGrp *staffGrp = dynamic_cast<StaffGrp *>(updatedParent);
                        staffGrp->ResetCommonPart();
                        staffGrp->ResetLabelsAddl();
                        staffGrp->ResetStaffgroupingsym();
                        staffGrp->ResetMidiinstrument();

                        // Instead of actually breaking up the staffgrp's tho (below), we're going to just reset the labels and the staffgrp symbol (above)
                        // so that the system doesn't register as being an actual part

                //        if (updatedParent->HasChild(updatedStaffDef, 0))
                //        {
                //            updatedParent->DetachChild(updatedParent->GetChildIndex(updatedStaffDef));
                //        }
                //        else
                //            LogWarning("Parent isn't child");

                //        updatedStaffDef->SetParent(mainStaffGrp);

                //        int index = mainStaffGrp->GetChildIndex(updatedParent);
                //        if (index == -1) {
                //            index = 0;
                //            LogWarning("StaffDef parent wasn't found!");
                //        }
                //        mainStaffGrp->InsertChild(updatedStaffDef, index);

                        changingPartObjs.push_back(updatedStaffDef);
                    }
                    else
                        LogWarning("StaffDef parent isn't staffgrp!2");
                }
            }
            else
                LogWarning("StaffDef parent isn't staffgrp!");
        }
        else
        {
            changingPartObjs.push_back(changingPartObj);
        }

        // Apply the labels, midiId, etc to the new staffs
        std::vector<Object *>::iterator changingPartObjIter;
        for (changingPartObjIter = changingPartObjs.begin(); changingPartObjIter != changingPartObjs.end(); changingPartObjIter++)
        {
            Object *changingObj = *changingPartObjIter;

            if (json.has<jsonxx::String>("name")) {
                std::string name = json.get<jsonxx::String>("name");
                AttCommonPart *att = dynamic_cast<AttCommonPart *>(changingObj);
                att->SetLabel(name);
            }

            if (json.has<jsonxx::String>("shortName")) {
                std::string shortName = json.get<jsonxx::String>("shortName");
                AttLabelsAddl *att = dynamic_cast<AttLabelsAddl *>(changingObj);
                att->SetLabelAbbr(shortName);
            }

            if (json.has<jsonxx::String>("id")) {
                std::string instrumentId = json.get<jsonxx::String>("id");
                AttMidiinstrument *att = dynamic_cast<AttMidiinstrument *>(changingObj);
                att->SetMidiInstrname(instrumentId);
            }

            if (json.has<jsonxx::Number>("midiProg")) {
                int midiProg = json.get<jsonxx::Number>("midiProg");
                AttMidiinstrument *att = dynamic_cast<AttMidiinstrument *>(changingObj);
                att->SetMidiInstrnum(midiProg);
            }
            // some instruments have multiple midiProg's, for now, just take the first one
            if (json.has<jsonxx::Array>("midiProg")) {
                jsonxx::Array midiProgArr = json.get<jsonxx::Array>("midiProg");
                if (midiProgArr.has<jsonxx::Number>(0)) {
                    int midiProg = midiProgArr.get<jsonxx::Number>(0);
                    AttMidiinstrument *att = dynamic_cast<AttMidiinstrument *>(changingObj);
                    att->SetMidiInstrnum(midiProg);
                }
            }
        }
    }

    m_doc.UnCastOffDoc();
    m_doc.CastOffDoc();
    return true;
#endif

    int octaveTransposition = m_transpose->GetPartTransposition(Transpose::Interval(), *m_doc.m_scoreDef.GetStaffDef(1),
                                                                71 - 12, 48 - 12, 77 - 12, 28 - 12, false);
    return false;
}

bool Toolkit::RenderToMidiFile(const std::string &filename)
{
    MidiFile outputfile;
    outputfile.absoluteTicks();
    m_doc.ExportMIDI(&outputfile);
    outputfile.sortTracks();
    outputfile.write(filename);

    return true;
}

int Toolkit::GetPageCount()
{
    return m_doc.GetPageCount();
}

int Toolkit::GetPageWithElement(const std::string &xmlId)
{
    Object *element = m_doc.FindChildByUuid(xmlId);
    if (!element) {
        return 0;
    }
    Page *page = dynamic_cast<Page *>(element->GetFirstParent(PAGE));
    if (!page) {
        return 0;
    }
    return page->GetIdx() + 1;
}

double Toolkit::GetTimeForElement(const std::string &xmlId)
{
    Object *element = m_doc.FindChildByUuid(xmlId);
    double timeofElement = 0.0;
    if (element->Is(NOTE)) {
        Note *note = dynamic_cast<Note *>(element);
        assert(note);
        timeofElement = note->m_playingOnset * 1000 / 120;
    }
    return timeofElement;
}

void Toolkit::SetCString(const std::string &data)
{
    if (m_cString) {
        free(m_cString);
        m_cString = NULL;
    }

    m_cString = (char *)malloc(strlen(data.c_str()) + 1);

    // something went wrong
    if (!m_cString) {
        return;
    }
    strcpy(m_cString, data.c_str());
}

void Toolkit::SetHumdrumBuffer(const char *data)
{
    if (m_humdrumBuffer) {
        free(m_humdrumBuffer);
        m_humdrumBuffer = NULL;
    }

#ifndef NO_HUMDRUM_SUPPORT
    hum::HumdrumFile file;
    file.readString(data);
    // apply Humdrum tools if there are any filters in the file.
    if (file.hasFilters()) {
        string output;
        hum::Tool_filter filter;
        filter.run(file);
        if (filter.hasHumdrumText()) {
            output = filter.getHumdrumText();
        }
        else {
            // humdrum structure not always correct in output from tools
            // yet, so reload.
            stringstream tempdata;
            tempdata << file;
            output = tempdata.str();
        }
        m_humdrumBuffer = (char *)malloc(output.size() + 1);
        if (!m_humdrumBuffer) {
            // something went wrong
            return;
        }
        strcpy(m_humdrumBuffer, output.c_str());
    }
    else {
        int size = (int)strlen(data) + 1;
        m_humdrumBuffer = (char *)malloc(size);
        if (!m_humdrumBuffer) {
            // something went wrong
            return;
        }
        strcpy(m_humdrumBuffer, data);
    }

#else
    size_t size = (int)strlen(data) + 1;
    m_humdrumBuffer = (char *)malloc(size);
    if (!m_humdrumBuffer) {
        // something went wrong
        return;
    }
    strcpy(m_humdrumBuffer, data);
#endif
}

const char *Toolkit::GetCString()
{
    if (m_cString) {
        return m_cString;
    }
    else {
        return "[unspecified]";
    }
}

const char *Toolkit::GetHumdrumBuffer()
{
    if (m_humdrumBuffer) {
        return m_humdrumBuffer;
    }
    else {
        return "[empty]";
    }
}

bool Toolkit::Drag(std::string elementId, int x, int y)
{
    if (!m_doc.GetDrawingPage()) return false;

    // Try to get the element on the current drawing page
    Object *element = m_doc.GetDrawingPage()->FindChildByUuid(elementId);

    // If it wasn't there, go back up to the whole doc
    if (!element) {
        element = m_doc.FindChildByUuid(elementId);
    }
    if (element->Is(NOTE)) {
        Note *note = dynamic_cast<Note *>(element);
        assert(note);
        Layer *layer = dynamic_cast<Layer *>(note->GetFirstParent(LAYER));
        if (!layer) return false;
        int oct;
        data_PITCHNAME pname
            = (data_PITCHNAME)m_view.CalculatePitchCode(layer, m_view.ToLogicalY(y), note->GetDrawingX(), &oct);
        note->SetPname(pname);
        note->SetOct(oct);
        return true;
    }
    return false;
}

bool Toolkit::Insert(std::string elementType, std::string startid, std::string endid)
{
    LogMessage("Insert!");
    if (!m_doc.GetDrawingPage()) return false;
    Object *start = m_doc.GetDrawingPage()->FindChildByUuid(startid);
    Object *end = m_doc.GetDrawingPage()->FindChildByUuid(endid);
    // Check if both start and end elements exist
    if (!start || !end) {
        LogMessage("Elements start and end ids '%s' and '%s' could not be found", startid.c_str(), endid.c_str());
        return false;
    }
    // Check if it is a LayerElement
    if (!dynamic_cast<LayerElement *>(start)) {
        LogMessage("Element '%s' is not supported as start element", start->GetClassName().c_str());
        return false;
    }
    if (!dynamic_cast<LayerElement *>(end)) {
        LogMessage("Element '%s' is not supported as end element", start->GetClassName().c_str());
        return false;
    }

    Measure *measure = dynamic_cast<Measure *>(start->GetFirstParent(MEASURE));
    assert(measure);
    if (elementType == "slur") {
        Slur *slur = new Slur();
        slur->SetStartid(startid);
        slur->SetEndid(endid);
        measure->AddChild(slur);
        m_doc.PrepareDrawing();
        return true;
    }
    return false;
}

bool Toolkit::Set(std::string elementId, std::string attrType, std::string attrValue)
{
    if (!m_doc.GetDrawingPage()) return false;
    Object *element = m_doc.GetDrawingPage()->FindChildByUuid(elementId);
    if (Att::SetCmn(element, attrType, attrValue)) return true;
    if (Att::SetCmnornaments(element, attrType, attrValue)) return true;
    if (Att::SetCritapp(element, attrType, attrValue)) return true;
    if (Att::SetExternalsymbols(element, attrType, attrValue)) return true;
    if (Att::SetMei(element, attrType, attrValue)) return true;
    if (Att::SetMensural(element, attrType, attrValue)) return true;
    if (Att::SetMidi(element, attrType, attrValue)) return true;
    if (Att::SetPagebased(element, attrType, attrValue)) return true;
    if (Att::SetShared(element, attrType, attrValue)) return true;
    return false;
}

bool Toolkit::TransposeKey(int newFifths)
{
    return m_transpose->transposeFifths(newFifths);
}

#ifdef USE_EMSCRIPTEN
bool Toolkit::ParseDragAction(jsonxx::Object param, std::string *elementId, int *x, int *y)
{
    if (!param.has<jsonxx::String>("elementId")) return false;
    (*elementId) = param.get<jsonxx::String>("elementId");
    if (!param.has<jsonxx::Number>("x")) return false;
    (*x) = param.get<jsonxx::Number>("x");
    if (!param.has<jsonxx::Number>("y")) return false;
    (*y) = param.get<jsonxx::Number>("y");
    return true;
}

bool Toolkit::ParseInsertAction(
    jsonxx::Object param, std::string *elementType, std::string *startid, std::string *endid)
{
    if (!param.has<jsonxx::String>("elementType")) return false;
    (*elementType) = param.get<jsonxx::String>("elementType");
    if (!param.has<jsonxx::String>("startid")) return false;
    (*startid) = param.get<jsonxx::String>("startid");
    if (!param.has<jsonxx::String>("endid")) return false;
    (*endid) = param.get<jsonxx::String>("endid");
    return true;
}

bool Toolkit::ParseSetAction(
    jsonxx::Object param, std::string *elementId, std::string *attrType, std::string *attrValue)
{
    if (!param.has<jsonxx::String>("elementId")) return false;
    (*elementId) = param.get<jsonxx::String>("elementId");
    if (!param.has<jsonxx::String>("attrType")) return false;
    (*attrType) = param.get<jsonxx::String>("attrType");
    if (!param.has<jsonxx::String>("attrValue")) return false;
    (*attrValue) = param.get<jsonxx::String>("attrValue");
    return true;
}

bool Toolkit::ParseTransposeKeyAction(
    jsonxx::Object param, int *newFifths)
{
    if (!param.has<jsonxx::Number>("newFifths")) return false;
    (*newFifths) = param.get<jsonxx::Number>("newFifths");
    return true;
}
#endif

} // namespace vrv
