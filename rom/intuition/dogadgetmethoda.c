/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.2  1996/11/08 11:28:00  aros
    All OS function use now Amiga types

    Moved intuition-driver protos to intuition_intern.h

    Revision 1.1  1996/10/25 14:17:54  aros
    New functions


    Desc: Implementation of DoGadgetMethodA
    Lang: english
*/
#include <exec/memory.h>
#include <intuition/classusr.h>
#include <intuition/gadgetclass.h>
#include <intuition/cghooks.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include "intuition_intern.h"

/*****************************************************************************

    NAME */
	#include <intuition/intuition.h>
	#include <clib/intuition_protos.h>

	AROS_LH4(IPTR, DoGadgetMethodA,

/*  SYNOPSIS */
	AROS_LHA(struct Gadget    *, gad, A0),
	AROS_LHA(struct Window    *, win, A1),
	AROS_LHA(struct Requester *, req, A2),
	AROS_LHA(Msg               , msg, A3),

/*  LOCATION */
	struct IntuitionBase *, IntuitionBase, 135, Intuition)

/*  FUNCTION
	Invokes a boopsi method on a object with a GadgetInfo derived from
	the supplied window or requester parameter.

    INPUTS
	gad - The gadget to work on
	win - The window which contains the gadget or the requester with
		the gadgets.
	req - If the gadget is in a requester, you must specify that one,
		too.
	message - Send this message to the gadget.

    RESULT
	The result depends on the contents of the message sent to the
	gadget.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS
	I have derived from a simular function from ClassAct where I have
	to "fake" the function which is not implemented under OS 2.04.
	There are likely a few differences between this routine and the
	real code, but this gets the job done.

	One thing to note, the Amiga Rom routinecauses some form of
	(layer?) locking. I presume the point of the lock is to avoid
	removing the gadget from the window durring a refresh, or to avoid
	resizing the window durring refresh, etc.

	This locking is fairly obvious within Workbench itself. When
	refreshing most any boopsi gadget(s) via RefreshGList() and you try
	to drag a Workbench icon you will get stuck in a layer lock.
	Workbench has a deadlock timer and is smart enough to release the
	lock and abort the drag. With this routine below this locking does
	not occur. Some might call it a good thing, however the issue
	should be revisited once more of Intuition has been implemented -
	if it hasn't been already?!. :)

    HISTORY
	29-10-95    digulla automatically created from
			    intuition_lib.fd and clib/intuition_protos.h
	25-10-96    calid   submitted the code

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)
    IPTR ret = 0;

    if (gad) /* OS routines work with NULL objects */
    {
	struct GadgetInfo *gi = (struct GadgetInfo *)AllocMem(sizeof(struct GadgetInfo), MEMF_PUBLIC|MEMF_CLEAR);

	if (gi)
	{
	    struct Window *tw;

	    if (req)
	    {
		gi->gi_Requester = req;
		tw = req->RWindow;
	    }
	    else
	    {
		gi->gi_Requester = NULL;
		tw = win;
	    } /* if */

	    if (tw)
	    {
		/* Initialize the GadgetInfo data. */
		gi->gi_Window	      = tw;
		gi->gi_Screen	      = tw->WScreen;
		gi->gi_RastPort       = tw->RPort;
		gi->gi_Layer	      = tw->WLayer;
		gi->gi_Pens.DetailPen = tw->DetailPen;
		gi->gi_Pens.BlockPen  = tw->BlockPen;
		gi->gi_DrInfo	      = GetScreenDrawInfo (gi->gi_Screen);

		switch (gad->GadgetType & GTYP_GTYPEMASK)
		{
		case GTYP_SCRGADGET:
		    gi->gi_Window	 = NULL;
		    gi->gi_Domain.Left	 = 0;
		    gi->gi_Domain.Top	 = 0;
		    gi->gi_Domain.Width  = tw->WScreen->Width;
		    gi->gi_Domain.Height = tw->WScreen->Height;

		    break;

		case GTYP_GZZGADGET:
		    gi->gi_Domain.Left	 = tw->BorderLeft;
		    gi->gi_Domain.Top	 = tw->BorderTop;
		    gi->gi_Domain.Width  = tw->Width  - tw->BorderLeft - tw->BorderRight;
		    gi->gi_Domain.Height = tw->Height - tw->BorderTop  - tw->BorderBottom;

		    break;

		case GTYP_REQGADGET:
		    gi->gi_Domain.Left	 = req->LeftEdge;
		    gi->gi_Domain.Top	 = req->TopEdge;
		    gi->gi_Domain.Width  = req->Width;
		    gi->gi_Domain.Height = req->Height;

		    break;

		default:
		    gi->gi_Domain.Left	 = 0;
		    gi->gi_Domain.Top	 = 0;
		    gi->gi_Domain.Width  = tw->Width;
		    gi->gi_Domain.Height = tw->Height;

		    break;

		} /* switch (gadgettype) */
	    } /* if (tw) */
	} /* if (gi) */

	switch (msg->MethodID)
	{
	case OM_NEW:
	case OM_SET:
	case OM_NOTIFY:
	case OM_UPDATE:
	    ((struct opSet *)msg)->ops_GInfo = gi;
	    ret = DoMethodA((Object *)gad, msg);
	    break;

	case GM_LAYOUT:
	    if (gi)
	    {
		((struct gpLayout *)msg)->gpl_GInfo = gi;

		ret = DoMethodA((Object *)gad, msg);
	    } /* if */
	    break;

	case GM_RENDER:
	    if (gi)
	    {
		struct RastPort *rp;

		/* Allocate a clone rastport derived from the GadgetInfo
		 * whose layer clipping information has been nulled out...
		 */
		rp = ObtainGIRPort(gi);

		if (rp)
		{
		    if (gi->gi_DrInfo)
		    {
			SetFont(rp, gi->gi_DrInfo->dri_Font);
		    } /* if */

		    ((struct gpRender *)msg)->gpr_RPort = rp;
		    ((struct gpRender *)msg)->gpr_GInfo = gi;

		    ret = DoMethodA((Object *)gad, msg);

		    ReleaseGIRPort(rp);
		} /* if */
	    } /* if */
	    break;

	default:
	    ((struct gpRender *)msg)->gpr_GInfo = gi;

	    ret = DoMethodA ((Object *)gad, msg);
	    break;

	} /* switch */

	if (gi)
	{
	    if (gi->gi_DrInfo)
		FreeScreenDrawInfo (gi->gi_Screen, gi->gi_DrInfo );

	    FreeMem ((APTR)gi, sizeof(struct GadgetInfo));
	} /* if */
    } /* if */

    return( (ULONG)ret );
    AROS_LIBFUNC_EXIT
} /* DoGadgetMethodA */
