/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Execute a CLI command
    Lang: English
*/

#include "dos_intern.h"
#include <dos/dosextens.h>
#include <utility/tagitem.h>

/*****************************************************************************

    NAME */
#include <proto/dos.h>

	AROS_LH3(LONG, Execute,

/*  SYNOPSIS */
	AROS_LHA(STRPTR, string, D1),
	AROS_LHA(BPTR  , input , D2),
	AROS_LHA(BPTR  , output, D3),

/*  LOCATION */
	struct DosLibrary *, DOSBase, 37, Dos)

/*  FUNCTION

    Execute a CLI command specified in 'string'. This string may contain
    features you may use on the shell commandline like redirection using >,
    < or >>. Execute() doesn't return until the command(s) that should be
    executed are finished.
        If 'input' is not NULL, more commands will be read from this stream
    until end of file is reached. 'output' will be used as the output stream
    of the commands (if output is not redirected). If 'output' is NULL the
    current window is used for output -- note that programs run from the
    Workbench doesn't normally have a current window.

    INPUTS

    string  --  pointer to a NULL-terminated string with commands
                (may be NULL)
    input   --  stream to use as input (may be NULL)
    output  --  stream to use as output (may be NULL)

    RESULT

    Boolean telling whether Execute() could find and start the specified
    command(s). (This is NOT the return code of the command(s).)

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    SystemTagList()

    INTERNALS

    To get the right result, the function ExecCommand() (used by both Execute()
    and SystemTagList()) uses NP_Synchronous to wait for the commands to
    finish. This is not the way AmigaOS does it as NP_Synchronous is not
    implemented (but defined).

    HISTORY
	27-11-96    digulla automatically created from
			    dos_lib.fd and clib/dos_protos.h
	2x-12-99    SDuvan  implemented

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct DosLibrary *, DOSBase)

    LONG           result;
    struct TagItem tags[] =
    {
        { SYS_Asynch,     FALSE        },
	{ SYS_Background, FALSE        },
	{ SYS_Input,      (IPTR)input  },
	{ SYS_Output,     (IPTR)output },
	{ SYS_Error,      (IPTR)NULL   },
	{ TAG_DONE,       0            }
    };


    result = SystemTagList(string, tags);

    if(result == 0)
	return DOSTRUE;
    else
	return DOSFALSE;

    AROS_LIBFUNC_EXIT
} /* Execute */

