/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <tools/shl.hxx>
#include <svx/svdview.hxx>
#include <svx/svdotext.hxx>
#include <svl/whiter.hxx>
#include <svx/fontwork.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <svl/itemiter.hxx>
#include <svl/srchitem.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/tbxcustomshapes.hxx>
#include <uitool.hxx>
#include <wview.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <docstat.hxx>
#include <IDocumentStatistics.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>

#include <svx/xtable.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/opengrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdundo.hxx>
#include <svx/xbtmpit.hxx>

#include "swundo.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "helpid.h"
#include "popup.hrc"
#include "shells.hrc"
#include "drwbassh.hxx"
#include "drawsh.hxx"

#define SwDrawShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"
#include "swabstdlg.hxx" //CHINA001
#include "misc.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SFX_IMPL_INTERFACE(SwDrawShell, SwDrawBaseShell, SW_RES(STR_SHELLNAME_DRAW))
{
	SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAW_POPUPMENU));
	SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_DRAW_TOOLBOX));
	SFX_CHILDWINDOW_REGISTRATION(SvxFontWorkChildWindow::GetChildWindowId());
}

TYPEINIT1(SwDrawShell,SwDrawBaseShell)

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


// #123922# check as the name implies
SdrObject* SwDrawShell::IsSingleFillableNonOLESelected()
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawView();

    if(!pSdrView)
    {
        return 0;
    }

    if(1 != pSdrView->GetMarkedObjectCount())
    {
        return 0;
    }

    SdrObject* pPickObj = pSdrView->GetMarkedObjectByIndex(0);

    if(!pPickObj)
    {
        return 0;
    }

    if(!pPickObj->IsClosedObj())
    {
        return 0;
    }

    if(dynamic_cast< SdrOle2Obj* >(pPickObj))
    {
        return 0;
    }

    return pPickObj;
}

// #123922# insert given graphic data dependent of the object type in focus
void SwDrawShell::InsertPictureFromFile(SdrObject& rObject)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pSdrView = rSh.GetDrawView();

    if(pSdrView)
    {
        SvxOpenGraphicDialog aDlg(SW_RESSTR(STR_INSERT_GRAPHIC));

        if(GRFILTER_OK == aDlg.Execute())
        {
            Graphic aGraphic;
            int nError(aDlg.GetGraphic(aGraphic));

            if(GRFILTER_OK == nError)
            {
                const bool bAsLink(aDlg.IsAsLink());
                SdrObject* pResult = &rObject;

                rSh.StartUndo(UNDO_PASTE_CLIPBOARD);

                if(dynamic_cast< SdrGrafObj* >(&rObject))
                {
                    SdrGrafObj* pNewGrafObj = (SdrGrafObj*)rObject.Clone();

                    pNewGrafObj->SetGraphic(aGraphic);

                    // #123922#  for handling MasterObject and virtual ones correctly, SW
                    // wants us to call ReplaceObject at the page, but that also
                    // triggers the same assertion (I tried it), so stay at the view method
                    pSdrView->ReplaceObjectAtView(&rObject, *pSdrView->GetSdrPageView(), pNewGrafObj);

                    // set in all cases - the Clone() will have copied an existing link (!)
                    pNewGrafObj->SetGraphicLink(
                        bAsLink ? aDlg.GetPath() : String(), 
                        bAsLink ? aDlg.GetCurrentFilter() : String());

                    pResult = pNewGrafObj;
                }
                else // if(rObject.IsClosedObj() && !dynamic_cast< SdrOle2Obj* >(&rObject))
                {
                    pSdrView->AddUndo(new SdrUndoAttrObj(rObject));

                    SfxItemSet aSet(pSdrView->GetModel()->GetItemPool(), XATTR_FILLSTYLE, XATTR_FILLBITMAP);

                    aSet.Put(XFillStyleItem(XFILL_BITMAP));
                    aSet.Put(XFillBitmapItem(String(), aGraphic));
                    rObject.SetMergedItemSetAndBroadcast(aSet);
                }

                rSh.EndUndo( UNDO_END );

                if(pResult)
                {
                    // we are done; mark the modified/new object
                    pSdrView->MarkObj(pResult, pSdrView->GetSdrPageView());
                }
            }
        }
    }
}

void SwDrawShell::Execute(SfxRequest &rReq)
{
	SwWrtShell			&rSh = GetShell();
	SdrView				*pSdrView = rSh.GetDrawView();
	const SfxItemSet	*pArgs = rReq.GetArgs();
	SfxBindings			&rBnd  = GetView().GetViewFrame()->GetBindings();
	sal_uInt16				 nSlotId = rReq.GetSlot();
	sal_Bool				 bChanged = pSdrView->GetModel()->IsChanged();

	pSdrView->GetModel()->SetChanged(sal_False);

	const SfxPoolItem* pItem;
	if(pArgs)
		pArgs->GetItemState(nSlotId, sal_False, &pItem);

    sal_Bool bMirror = sal_True;

	switch (nSlotId)
	{
		case SID_OBJECT_ROTATE:
			if (rSh.IsObjSelected() && pSdrView->IsRotateAllowed())
			{
				if (GetView().IsDrawRotate())
					rSh.SetDragMode(SDRDRAG_MOVE);
				else
					rSh.SetDragMode(SDRDRAG_ROTATE);

				GetView().FlipDrawRotate();
			}
			break;

		case SID_BEZIER_EDIT:
			if (GetView().IsDrawRotate())
			{
				rSh.SetDragMode(SDRDRAG_MOVE);
				GetView().FlipDrawRotate();
			}
			GetView().FlipDrawSelMode();
			pSdrView->SetFrameDragSingles(GetView().IsDrawSelMode());
			GetView().AttrChangedNotify(&rSh); // Shellwechsel...
			break;

		case SID_OBJECT_HELL:
			if (rSh.IsObjSelected())
			{
				rSh.StartUndo( UNDO_START );
				SetWrapMode(FN_FRAME_WRAPTHRU_TRANSP);
				rSh.SelectionToHell();
				rSh.EndUndo( UNDO_END );
				rBnd.Invalidate(SID_OBJECT_HEAVEN);
			}
			break;

		case SID_OBJECT_HEAVEN:
			if (rSh.IsObjSelected())
			{
				rSh.StartUndo( UNDO_START );
				SetWrapMode(FN_FRAME_WRAPTHRU);
				rSh.SelectionToHeaven();
				rSh.EndUndo( UNDO_END );
				rBnd.Invalidate(SID_OBJECT_HELL);
			}
			break;

		case FN_TOOL_HIERARCHIE:
			if (rSh.IsObjSelected())
			{
				rSh.StartUndo( UNDO_START );
				if (rSh.GetLayerId() == 0)
				{
					SetWrapMode(FN_FRAME_WRAPTHRU);
					rSh.SelectionToHeaven();
				}
				else
				{
					SetWrapMode(FN_FRAME_WRAPTHRU_TRANSP);
					rSh.SelectionToHell();
				}
				rSh.EndUndo( UNDO_END );
                rBnd.Invalidate( SID_OBJECT_HELL );
                rBnd.Invalidate( SID_OBJECT_HEAVEN );
			}
			break;

        case SID_FLIP_VERTICAL:
            bMirror = sal_False;
            /* no break */
        case SID_FLIP_HORIZONTAL:
            rSh.MirrorSelection( bMirror );
            break;

		case SID_FONTWORK:
		{
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &rSh.GetView()));
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
			if (pArgs)
			{
				pVFrame->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
					((const SfxBoolItem&)(pArgs->Get(SID_FONTWORK))).GetValue());
			}
			else
				pVFrame->ToggleChildWindow( SvxFontWorkChildWindow::GetChildWindowId() );
			pVFrame->GetBindings().Invalidate(SID_FONTWORK);
		}
		break;
		case FN_FORMAT_FOOTNOTE_DLG:
		{
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

            VclAbstractDialog* pDlg = pFact->CreateSwFootNoteOptionDlg( GetView().GetWindow(), GetView().GetWrtShell(), DLG_DOC_FOOTNOTE );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
			pDlg->Execute();
			delete pDlg;
			break;
		}
		case FN_NUMBERING_OUTLINE_DLG:
		{
			SfxItemSet aTmp(GetPool(), FN_PARAM_1, FN_PARAM_1);
			SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            SfxAbstractTabDialog* pDlg = pFact->CreateSwTabDialog( DLG_TAB_OUTLINE,
														GetView().GetWindow(), &aTmp, GetView().GetWrtShell());
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
			pDlg->Execute();
			delete pDlg;
			rReq.Done();
		}
		break;
		case SID_OPEN_XML_FILTERSETTINGS:
		{
			try
			{
				uno::Reference < ui::dialogs::XExecutableDialog > xDialog(::comphelper::getProcessServiceFactory()->createInstance(rtl::OUString::createFromAscii("com.sun.star.comp.ui.XSLTFilterDialog")), uno::UNO_QUERY);
				if( xDialog.is() )
				{
					xDialog->execute();
				}
			}
			catch( uno::Exception& )
			{
			}
			rReq.Ignore ();
		}
		break;
		case FN_WORDCOUNT_DIALOG:
		{
			SwDocStat aCurr;
			SwDocStat aDocStat( rSh.getIDocumentStatistics()->GetDocStat() );
			{
				SwWait aWait( *GetView().GetDocShell(), true );
				rSh.StartAction();
				rSh.CountWords( aCurr );
				rSh.UpdateDocStat( aDocStat );
				rSh.EndAction();
			}

			SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
			DBG_ASSERT(pFact, "Dialogdiet fail!");
			AbstractSwWordCountDialog* pDialog = pFact->CreateSwWordCountDialog( GetView().GetWindow() );
			pDialog->SetValues(aCurr, aDocStat );
			pDialog->Execute();
			delete pDialog;
		}
		break;
		case SID_EXTRUSION_TOOGLE:
		case SID_EXTRUSION_TILT_DOWN:
		case SID_EXTRUSION_TILT_UP:
		case SID_EXTRUSION_TILT_LEFT:
		case SID_EXTRUSION_TILT_RIGHT:
		case SID_EXTRUSION_3D_COLOR:
		case SID_EXTRUSION_DEPTH:
		case SID_EXTRUSION_DIRECTION:
		case SID_EXTRUSION_PROJECTION:
		case SID_EXTRUSION_LIGHTING_DIRECTION:
		case SID_EXTRUSION_LIGHTING_INTENSITY:
		case SID_EXTRUSION_SURFACE:
		case SID_EXTRUSION_DEPTH_FLOATER:
		case SID_EXTRUSION_DIRECTION_FLOATER:
		case SID_EXTRUSION_LIGHTING_FLOATER:
		case SID_EXTRUSION_SURFACE_FLOATER:
		case SID_EXTRUSION_DEPTH_DIALOG:
			svx::ExtrusionBar::execute( pSdrView, rReq, rBnd );
			rReq.Ignore ();
			break;

		case SID_FONTWORK_SHAPE:
		case SID_FONTWORK_SHAPE_TYPE:
		case SID_FONTWORK_ALIGNMENT:
		case SID_FONTWORK_SAME_LETTER_HEIGHTS:
		case SID_FONTWORK_CHARACTER_SPACING:
		case SID_FONTWORK_KERN_CHARACTER_PAIRS:
		case SID_FONTWORK_CHARACTER_SPACING_FLOATER:
		case SID_FONTWORK_ALIGNMENT_FLOATER:
		case SID_FONTWORK_CHARACTER_SPACING_DIALOG:
			svx::FontworkBar::execute( pSdrView, rReq, rBnd );
			rReq.Ignore ();
			break;

        case SID_INSERT_GRAPHIC:
        {
            // #123922# check if we can do something
            SdrObject* pObj = IsSingleFillableNonOLESelected();

            if(pObj)
            {
                // ...and if yes, do something
                InsertPictureFromFile(*pObj);
                bool bBla = true;
            }

            break;
        }

		default:
			DBG_ASSERT(!this, "falscher Dispatcher");
			return;
	}
	if (pSdrView->GetModel()->IsChanged())
		rSh.SetModified();
	else if (bChanged)
		pSdrView->GetModel()->SetChanged(sal_True);
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDrawShell::GetState(SfxItemSet& rSet)
{
	SwWrtShell &rSh = GetShell();
	SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
	SfxWhichIter aIter( rSet );
	sal_uInt16 nWhich = aIter.FirstWhich();
	sal_Bool bProtected = rSh.IsSelObjProtected(FLYPROTECT_CONTENT);

	if (!bProtected)	// Im Parent nachsehen
		bProtected |= rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT ) != 0;

	while( nWhich )
	{
		switch( nWhich )
		{
			case SID_OBJECT_HELL:
				if ( !rSh.IsObjSelected() || rSh.GetLayerId() == 0 || bProtected )
					rSet.DisableItem( nWhich );
				break;

			case SID_OBJECT_HEAVEN:
				if ( !rSh.IsObjSelected() || rSh.GetLayerId() == 1 || bProtected )
					rSet.DisableItem( nWhich );
				break;

			case FN_TOOL_HIERARCHIE:
				if ( !rSh.IsObjSelected() || bProtected )
					rSet.DisableItem( nWhich );
				break;

			case SID_OBJECT_ROTATE:
			{
				const sal_Bool bIsRotate = GetView().IsDrawRotate();
				if ( (!bIsRotate && !pSdrView->IsRotateAllowed()) || bProtected )
					rSet.DisableItem( nWhich );
				else
					rSet.Put( SfxBoolItem( nWhich, bIsRotate ) );
			}
			break;

			case SID_BEZIER_EDIT:
				if (!Disable(rSet, nWhich))
					rSet.Put( SfxBoolItem( nWhich, !GetView().IsDrawSelMode()));
			break;

            case SID_FLIP_VERTICAL:
                if ( !pSdrView->IsMirrorAllowed() || bProtected )
                {
                    rSet.DisableItem( nWhich );
                }
                else
                {
                    // TTTT - needs to be adapted in aw080:
                    // state is not kept for drawing objects --> provide not flipped state
                    rSet.Put( SfxBoolItem( nWhich, sal_False ) );
                }
                break;

            case SID_FLIP_HORIZONTAL:
                if ( !pSdrView->IsMirrorAllowed() || bProtected )
                {
                    rSet.DisableItem( nWhich );
                }
                else
                {
                    // TTTT - needs to be adapted in aw080:
                    // state is not kept for drawing objects --> provide not flipped state
                    rSet.Put( SfxBoolItem( nWhich, sal_False ) );
                }
                break;

			case SID_FONTWORK:
			{
				if (bProtected)
					rSet.DisableItem( nWhich );
				else
				{
					const sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();
					rSet.Put(SfxBoolItem( nWhich , GetView().GetViewFrame()->HasChildWindow(nId)));
				}
			}
			break;

            case SID_INSERT_GRAPHIC:
            {
                // #123922# check if we can do something
                SdrObject* pObj = IsSingleFillableNonOLESelected();

                if(!pObj)
                {
                    rSet.DisableItem(nWhich);
                }

                break;
            }
		}
		nWhich = aIter.NextWhich();
	}
	svx::ExtrusionBar::getState( pSdrView, rSet );
	svx::FontworkBar::getState( pSdrView, rSet );
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/



SwDrawShell::SwDrawShell(SwView &_rView) :
    SwDrawBaseShell(_rView)
{
	SetHelpId(SW_DRAWSHELL);
	SetName(String::CreateFromAscii("Draw"));

    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Draw));
}

/*************************************************************************
|*
|* SfxRequests fuer FontWork bearbeiten
|*
\************************************************************************/



void SwDrawShell::ExecFormText(SfxRequest& rReq)
{
	SwWrtShell &rSh = GetShell();
	SdrView*	pDrView = rSh.GetDrawView();
	sal_Bool		bChanged = pDrView->GetModel()->IsChanged();
	pDrView->GetModel()->SetChanged(sal_False);

	const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

	if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
	{
		const SfxItemSet& rSet = *rReq.GetArgs();

		if ( pDrView->IsTextEdit() )
		{
			pDrView->SdrEndTextEdit( sal_True );
			GetView().AttrChangedNotify(&rSh);
		}

		pDrView->SetAttributes(rSet);
	}
	if (pDrView->GetModel()->IsChanged())
		rSh.SetModified();
	else
		if (bChanged)
			pDrView->GetModel()->SetChanged(sal_True);
}

/*************************************************************************
|*
|* Statuswerte fuer FontWork zurueckgeben
|*
\************************************************************************/



void SwDrawShell::GetFormTextState(SfxItemSet& rSet)
{
	SwWrtShell &rSh = GetShell();
	SdrView* pDrView = rSh.GetDrawView();
	const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
	const SdrObject* pObj = NULL;
	SvxFontWorkDialog* pDlg = NULL;

	const sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();

	SfxViewFrame* pVFrame = GetView().GetViewFrame();
	if ( pVFrame->HasChildWindow(nId) )
		pDlg = (SvxFontWorkDialog*)(pVFrame->GetChildWindow(nId)->GetWindow());

	if ( rMarkList.GetMarkCount() == 1 )
		pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    const SdrTextObj* pTextObj = dynamic_cast< const SdrTextObj* >(pObj);
    const bool bDeactivate(
        !pObj ||
        !pTextObj ||
        !pTextObj->HasText() ||
        dynamic_cast< const SdrObjCustomShape* >(pObj)); // #121538# no FontWork for CustomShapes

    if(bDeactivate)
	{
        rSet.DisableItem(XATTR_FORMTXTSTYLE);
        rSet.DisableItem(XATTR_FORMTXTADJUST);
        rSet.DisableItem(XATTR_FORMTXTDISTANCE);
        rSet.DisableItem(XATTR_FORMTXTSTART);
        rSet.DisableItem(XATTR_FORMTXTMIRROR);
        rSet.DisableItem(XATTR_FORMTXTHIDEFORM);
        rSet.DisableItem(XATTR_FORMTXTOUTLINE);
        rSet.DisableItem(XATTR_FORMTXTSHADOW);
        rSet.DisableItem(XATTR_FORMTXTSHDWCOLOR);
        rSet.DisableItem(XATTR_FORMTXTSHDWXVAL);
        rSet.DisableItem(XATTR_FORMTXTSHDWYVAL);
	}
	else
	{
		if ( pDlg )
			pDlg->SetColorTable(XColorList::GetStdColorList());

		pDrView->GetAttributes( rSet );
	}
}
