/////////////////////////////////////////////////////////////////////////////
// Name:        hairpin.h
// Author:      Laurent Pugin
// Created:     2016
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#ifndef __VRV_HAIRPIN_H__
#define __VRV_HAIRPIN_H__

#include "atts_cmn.h"
#include "controlelement.h"
#include "timeinterface.h"

namespace vrv {

//----------------------------------------------------------------------------
// Hairpin
//----------------------------------------------------------------------------

class Hairpin : public ControlElement,
                public TimeSpanningInterface,
                public AttColor,
                public AttHairpinLog,
                public AttPlacement,
                public AttVerticalAlignment {
public:
    /**
     * @name Constructors, destructors, and other standard methods
     * Reset method reset all attribute classes
     */
    ///@{
    Hairpin();
    virtual ~Hairpin();
    virtual void Reset();
    virtual std::string GetClassName() const { return "Hairpin"; }
    virtual ClassId GetClassId() const { return HAIRPIN; }
    ///@}

    /**
     * @name Getter to interfaces
     */
    ///@{
    virtual TimePointInterface *GetTimePointInterface() { return dynamic_cast<TimePointInterface *>(this); }
    virtual TimeSpanningInterface *GetTimeSpanningInterface() { return dynamic_cast<TimeSpanningInterface *>(this); }
    ///@}

    /**
     * @name Setter and getter for left and right links
     */
    ///@{
    void SetLeftLink(ControlElement *leftLink);
    ControlElement *GetLeftLink() { return m_leftLink; }
    void SetRightLink(ControlElement *rightLink);
    ControlElement *GetRightLink() { return m_rightLink; }
    ///@}

    //----------//
    // Functors //
    //----------//

    /**
     * See Object::PrepareFloatingGrps
     */
    virtual int PrepareFloatingGrps(FunctorParams *functoParams);

    /**
     * See Object::ResetDrawing
     */
    virtual int ResetDrawing(FunctorParams *functorParams);

protected:
    //
private:
    //
public:
    //
private:
    /**
     * A pointer to the possible left link of the Hairpin.
     * This is either another Hairpin or a Dynam that ends / appears at the same position.
     */
    ControlElement *m_leftLink;
                    
    /**
     * A pointer to the possible right link of the Hairpin.
     * This is either another Hairpin or a Dynam that starts / appears at the same position.
     */
    ControlElement *m_rightLink;
};

} // namespace vrv

#endif
