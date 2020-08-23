//////////////////////////////////////////////////////////////////////////////
//
//  vstar_pivot_offsets.cpp
//
//  Description:
//      Contains Unigraphics entry points for the application.
//
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE 1

//  Include files
#include <uf.h>
#include <uf_defs.h>
#include <uf_exit.h>
#include <uf_ui.h>

#include <uf_styler.h>
#include <uf_mb.h>
#include <uf_mom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/*
#if ! defined ( __hp9000s800 ) && ! defined ( __sgi ) && ! defined ( __sun )
#	include <strstream>
	using std::ostrstream;
	using std::endl;
	using std::ends;
#else
#	include <strstream.h>
#endif
#include <iostream.h>
*/

#include "pivot_offsets.h"

/* The following definition defines the number of callback entries */
/* in the callback structure:                                      */
/* UF_STYLER_callback_info_t CS_cbs */
#define CS_CB_COUNT ( 7 + 1 ) /* Add 1 for the terminator */

/*--------------------------------------------------------------------------
The following structure defines the callback entries used by the
styler file.  This structure MUST be passed into the user function,
UF_STYLER_create_dialog along with CS_CB_COUNT.
--------------------------------------------------------------------------*/
static UF_STYLER_callback_info_t CS_cbs[CS_CB_COUNT] =
{
 {UF_STYLER_DIALOG_INDEX, UF_STYLER_CONSTRUCTOR_CB  , 0, CS_construct_cb},
 {UF_STYLER_DIALOG_INDEX, UF_STYLER_OK_CB           , 0, CS_ok_cb},
 {UF_STYLER_DIALOG_INDEX, UF_STYLER_CANCEL_CB       , 0, CS_cancel_cb},
 {CS_OPTION_CORNER      , UF_STYLER_ACTIVATE_CB     , 0, CS_corner_head_cb},
 {CS_TOGGLE_SPECIAL     , UF_STYLER_VALUE_CHANGED_CB, 0, CS_special_cb},
 {CS_L                  , UF_STYLER_ACTIVATE_CB     , 0, CS_real_cb},
 {CS_P                  , UF_STYLER_ACTIVATE_CB     , 0, CS_real_cb},
 {UF_STYLER_NULL_OBJECT, UF_STYLER_NO_CB, 0, 0 }
};



/*--------------------------------------------------------------------------
UF_MB_styler_actions_t contains 4 fields.  These are defined as follows:

Field 1 : the name of your dialog that you wish to display.
Field 2 : any client data you wish to pass to your callbacks.
Field 3 : your callback structure.
Field 4 : flag to inform menubar of your dialog location.  This flag MUST
          match the resource set in your dialog!  Do NOT ASSUME that changing
          this field will update the location of your dialog.  Please use the
          UIStyler to indicate the position of your dialog.
--------------------------------------------------------------------------*/
static UF_MB_styler_actions_t actions[] = {
    { "pivot_offsets.dlg",  NULL,   CS_cbs,  UF_MB_STYLER_IS_NOT_TOP },
    { NULL,  NULL,  NULL,  0 } /* This is a NULL terminated list */
};


char *temp_file="pivot_offsets.syslog\0";

int first_index = 0; // 0..NUM
int check_index = 0; // 0 1

double param_l = 0;
double param_p = 0;

const int NUM = 4 ;
double Param[NUM][2]={ {184.885 , 33 }, {214.906 , 33} , { 245.000 , 33 } , {274.959 , 33} } ;

int work_file (int t,char *filename);
int ufusr_main( char * );
int _construct_cb ( int dialog_id );
int _ok_cb ( int dialog_id );
int _option_corner ( int dialog_id );
int _toggle_sp ( int dialog_id );


//----------------------------------------------------------------------------
//  Activation Methods
//----------------------------------------------------------------------------

//  Explicit Activation
//      This entry point is used to activate the application explicitly, as in
//      "File->Execute UG/Open->User Function..."
extern "C" DllExport void ufusr( char *parm, int *returnCode, int rlen )
{
    /* Initialize the API environment */
    int errorCode = UF_initialize();

    if ( 0 == errorCode )
    {
        /* TODO: Add your application code here */
        ufusr_main( parm ) ;

        /* Terminate the API environment */
        errorCode = UF_terminate();
    }

    /* Print out any error messages */
    *returnCode=0;
}

//----------------------------------------------------------------------------
//  Utilities
//----------------------------------------------------------------------------

// Unload Handler
//     This function specifies when to unload your application from Unigraphics.
//     If your application registers a callback (from a MenuScript item or a
//     User Defined Object for example), this function MUST return
//     "UF_UNLOAD_UG_TERMINATE".
extern "C" int ufusr_ask_unload( void )
{
     /* unload immediately after application exits*/
     return ( UF_UNLOAD_IMMEDIATELY );

     /*via the unload selection dialog... */
     /*return ( UF_UNLOAD_SEL_DIALOG );   */
     /*when UG terminates...              */
     /*return ( UF_UNLOAD_UG_TERMINATE ); */
}

/*--------------------------------------------------------------------------
You have the option of coding the cleanup routine to perform any housekeeping
chores that may need to be performed.  If you code the cleanup routine, it is
automatically called by Unigraphics.
--------------------------------------------------------------------------*/
extern void ufusr_cleanup (void)
{
    return;
}



/*********************************************
 t = 0 - read
 t = 1 - write
*********************************************/
int work_file (int t,char *filename)
{
  FILE *fp ;
  char pathfile[255];
  char *ptr;

  pathfile[0]='\0';
  UF_translate_variable("UGII_TMP_DIR", &ptr);
  if (ptr==NULL) {
    sprintf(pathfile,"%s",filename);
  } else {
    sprintf(pathfile,"%s\\%s",ptr,filename);
  }

	if (t==0) {
		fp=fopen(pathfile,"r");
	  if (fp==NULL) return(-1);
	  fscanf(fp,"%d  %d  %lf  %lf ",&first_index,&check_index,&param_l,&param_p);
	}

	if (first_index>=NUM) first_index=NUM-1;
	if (first_index<0) first_index=0;
	if (check_index<0) check_index=0;
	if (check_index>1) check_index=1;

	if (t==1) {
		fp=fopen(pathfile,"w");
	  if (fp==NULL) return(-1);
	  fprintf(fp,"%d  %d  %.5f  %.5f ",first_index,check_index,param_l,param_p);
	}

	fclose(fp);

	return(0);
}


/*********************************************



*********************************************/
int ufusr_main( char *param )
{
  int errorCode;

  UF_MOM_id_t   mom ; //UF_MOM_id_t mom_id = ( UF_MOM_id_t ) client_data ;
  void *interp = NULL ;

  const char *valuepath;
  char pathfile[255];
  char *ptr;

  int response ;
  int module_id;

  UF_ask_application_module(&module_id);
  if (UF_APP_CAM!=module_id) {
    uc1601("Запуск DLL - производится из модуля CAM - MOM\n...DLL terminated...",1);
    return(0) ;
  }

  /* Get the TCL interpreter id from the ufusr param */
  //(void)UF_MOM_ask_interp_from_param ( param, &interp ) ;
  errorCode=UF_MOM_ask_interp_from_param ( param, &interp ) ;
  if ( 0 != errorCode ) {
    uc1601 ("Don't get the TCL interpreter id from the ufusr param\nDLL terminated...", TRUE );
    return(0) ;
  }
  /* Get the MOM id from the ufusr param */
  errorCode=UF_MOM_ask_mom (param, &mom) ;
  if ( 0 != errorCode ) {
    uc1601 ("Don't get the MOM id from the ufusr param\nDLL terminated...", TRUE );
    return(0) ;
  }

  /****************************Чтение переменных из mom****************************/
  UF_print_syslog(" ufusr_main DLL variables -> read - mom_load_dir_dlg \n",FALSE);

  /* set up new path */
  /*Полный путь до файла диалога*/
  UF_MOM_ask_string ( mom,"load_dir_dlg",&valuepath);
  if (NULL==valuepath) {
       UF_translate_variable("UGII_CAM_POST_DIR", &ptr);
       if (ptr==NULL) {
         UF_print_syslog("Ошибка : переменная UGII_CAM_POST_DIR не определена\n",FALSE);
         pathfile[0]='\0';  sprintf(pathfile,"%s",actions->styler_file);
       } else {
         pathfile[0]='\0';  sprintf(pathfile,"%s%s",ptr,actions->styler_file);
       }
  } else {
       pathfile[0]='\0';  sprintf(pathfile,"%s",valuepath);
  }
  //uc1601(pathfile,1) ;
  printf("\n mom_load_dir_dlg=%s ",pathfile);

  work_file(0,temp_file);

  /****************************************************************************/

     if ( ( errorCode = UF_STYLER_create_dialog (
            pathfile,
           CS_cbs,      /* Callbacks from dialog */
           CS_CB_COUNT, /* number of callbacks*/
            NULL,        /* This is your client data */
            &response ) ) != 0 )
    {
       //uc1601("Ошибка загрузки шаблона диалога\n -Ошибка памяти- \n, 2006.",1);
       UF_print_syslog(" ufusr_main DLL - Ошибка загрузки шаблона диалога\n -Ошибка памяти- \n",FALSE);
       //return (-1);
    }

  /****************************************************************************/

  work_file(1,temp_file);

  /***********************Запись переменных в mom*********************************/
  UF_print_syslog(" ufusr_main DLL variables ->write \n",FALSE);

  /*mom_*/
  UF_MOM_set_double ( mom,"param_l",param_l );
  UF_MOM_set_double ( mom,"param_p",param_p );

  return(0);
}

int _construct_cb ( int dialog_id )
{
 UF_STYLER_item_value_type_t data  ;
 int err;

  data.item_attr=UF_STYLER_VALUE;

  data.item_id=CS_TOGGLE_SPECIAL;
  data.value.integer=check_index;
  err=UF_STYLER_set_value(dialog_id,&data);

  data.item_id=CS_OPTION_CORNER;
  data.value.integer=first_index;
  data.subitem_index=first_index;
  err=UF_STYLER_set_value(dialog_id,&data);

  if (check_index==0) {
  	param_l = Param[first_index][0];
  	param_p = Param[first_index][1];
  }

  data.item_id=CS_L;
  data.value.real=param_l ;
  err=UF_STYLER_set_value(dialog_id,&data);

  data.item_id=CS_P;
  data.value.real=param_p ;
  err=UF_STYLER_set_value(dialog_id,&data);

  UF_STYLER_free_value (&data) ;

   _toggle_sp ( dialog_id ) ;

  return (0);
}


int _toggle_sp ( int dialog_id )
{
  UF_STYLER_item_value_type_t data  ;
  int i;

  data.item_attr=UF_STYLER_VALUE;
  data.item_id=CS_TOGGLE_SPECIAL;
  UF_STYLER_ask_value(dialog_id,&data);
  i=data.value.integer;

  data.item_attr=UF_STYLER_SENSITIVITY;

  data.item_id=CS_OPTION_CORNER;
  data.value.integer=(int)(!i);
  UF_STYLER_set_value(dialog_id,&data);

  data.item_id=CS_L;
  data.value.integer=i;
  UF_STYLER_set_value(dialog_id,&data);

  data.item_id=CS_P;
  data.value.integer=i;
  UF_STYLER_set_value(dialog_id,&data);

  UF_STYLER_free_value (&data) ;

  return (0);
}

int _option_corner ( int dialog_id )
{
  UF_STYLER_item_value_type_t data  ;
  int i;

  data.item_attr=UF_STYLER_VALUE;

  data.item_id=CS_OPTION_CORNER;
  UF_STYLER_ask_value(dialog_id,&data);
  i=data.value.integer;

  if (i>=NUM) i=NUM-1;
  if (i<0) i=0 ;

  data.item_id=CS_L;
  data.value.real=Param[i][0];
  UF_STYLER_set_value(dialog_id,&data);

  data.item_id=CS_P;
  data.value.real=Param[i][1];
  UF_STYLER_set_value(dialog_id,&data);

  UF_STYLER_free_value (&data) ;

  return (0);
}


int _ok_cb ( int dialog_id )
{
  UF_STYLER_item_value_type_t data  ;

  data.item_attr=UF_STYLER_VALUE;

  data.item_id=CS_OPTION_CORNER;
  UF_STYLER_ask_value(dialog_id,&data);
  first_index=data.value.integer;

  data.item_id=CS_TOGGLE_SPECIAL;
  UF_STYLER_ask_value(dialog_id,&data);
  check_index=data.value.integer;

  data.item_id=CS_L;
  UF_STYLER_ask_value(dialog_id,&data);
  param_l=data.value.real;

  data.item_id=CS_P;
  UF_STYLER_ask_value(dialog_id,&data);
  param_p=data.value.real;

  UF_STYLER_free_value (&data) ;

  return (0);
}



/*-------------------------------------------------------------------------*/
/*---------------------- UIStyler Callback Functions ----------------------*/
/*-------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------
 * Callback Name: CS_construct_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CS_construct_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */
     _construct_cb ( dialog_id ) ;

     UF_terminate ();

    /* Callback acknowledged, do not terminate dialog */
    return (UF_UI_CB_CONTINUE_DIALOG);
    /* A return value of UF_UI_CB_EXIT_DIALOG will not be accepted    */
    /* for this callback type.  You must continue dialog construction.*/

}


/* -------------------------------------------------------------------------
 * Callback Name: CS_ok_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CS_ok_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */
     _ok_cb ( dialog_id ) ;

     UF_terminate ();

    /* Callback acknowledged, terminate dialog             */
    /* It is STRONGLY recommended that you exit your       */
    /* callback with UF_UI_CB_EXIT_DIALOG in a ok callback.*/
    /* return ( UF_UI_CB_EXIT_DIALOG );                    */
    return (UF_UI_CB_EXIT_DIALOG);

}


/* -------------------------------------------------------------------------
 * Callback Name: CS_cancel_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CS_cancel_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */
     _ok_cb ( dialog_id ) ;

     UF_terminate ();

    /* Callback acknowledged, terminate dialog             */
    /* It is STRONGLY recommended that you exit your       */
    /* callback with UF_UI_CB_EXIT_DIALOG in a cancel call */
    /* back rather than UF_UI_CB_CONTINUE_DIALOG.          */
    return ( UF_UI_CB_EXIT_DIALOG );

}


/* -------------------------------------------------------------------------
 * Callback Name: CS_corner_head_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CS_corner_head_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */
     _option_corner ( dialog_id ) ;

     UF_terminate ();

    /* Callback acknowledged, do not terminate dialog */
    return (UF_UI_CB_CONTINUE_DIALOG);

    /* or Callback acknowledged, terminate dialog.    */
    /* return ( UF_UI_CB_EXIT_DIALOG );               */

}


/* -------------------------------------------------------------------------
 * Callback Name: CS_special_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CS_special_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */
     _toggle_sp ( dialog_id ) ;

     UF_terminate ();

    /* Callback acknowledged, do not terminate dialog */
    return (UF_UI_CB_CONTINUE_DIALOG);

    /* or Callback acknowledged, terminate dialog.  */
    /* return ( UF_UI_CB_EXIT_DIALOG );             */

}


/* -------------------------------------------------------------------------
 * Callback Name: CS_real_cb
 * This is a callback function associated with an action taken from a
 * UIStyler object.
 *
 * Input: dialog_id   -   The dialog id indicate which dialog this callback
 *                        is associated with.  The dialog id is a dynamic,
 *                        unique id and should not be stored.  It is
 *                        strictly for the use in the NX Open API:
 *                               UF_STYLER_ask_value(s)
 *                               UF_STYLER_set_value
 *        client_data -   Client data is user defined data associated
 *                        with your dialog.  Client data may be bound
 *                        to your dialog with UF_MB_add_styler_actions
 *                        or UF_STYLER_create_dialog.
 *        callback_data - This structure pointer contains information
 *                        specific to the UIStyler Object type that
 *                        invoked this callback and the callback type.
 * -----------------------------------------------------------------------*/
int CS_real_cb ( int dialog_id,
             void * client_data,
             UF_STYLER_item_value_type_p_t callback_data)
{
     /* Make sure User Function is available. */
     if ( UF_initialize() != 0)
          return ( UF_UI_CB_CONTINUE_DIALOG );

     /* ---- Enter your callback code here ----- */

     UF_terminate ();

    /* Callback acknowledged, do not terminate dialog */
    return (UF_UI_CB_CONTINUE_DIALOG);

    /* or Callback acknowledged, terminate dialog.    */
    /* return ( UF_UI_CB_EXIT_DIALOG );               */

}
