// $Id: resource.h 12 2005-08-14 01:42:51Z Rasqual $

// *
// *   gryff, a GRF archive management utility, src/resource.h
// *   Copyright (C) 2003, 2004 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

// * resource.h
// * Contains resource constant definitions.
// *
// *@author $Author: Rasqual $
// *@version $Revision: 12 $
// *

#ifndef IDC_STATIC
# define IDC_STATIC                                -1
#endif
#define  IDM_IDS_OFFSET                           3000


// Multiple resource: Main frame
#define IDC_GRYFF                                  100
// CFG Strings: Registry
#define IDS_APPLICATION_REGISTRY_KEY               101
// Multiple resource: MDI child frame
#define IDC_GRYFFCHILD                             102
// Extended accelerator table for keys that clash with focused child controls
#define IDC_GRYFF_ACCEL_EX                         103

// Application icons
#define IDI_OPENFOLDER                             504
#define IDI_CLOSEDFOLDER                           505
#define IDI_FILEICON                               506


// Command IDs: Main frame / MDI frame menu items
#define IDM_FILE_NEW                               701
#define IDM_FILE_NEW_FROM_GRF                      702
#define IDM_FILE_OPEN                              703
#define IDM_FILE_CLOSE                             704
#define IDM_FILE_CLOSE_ALL_BUT_CURRENT             705
#define IDM_FILE_SAVE                              706
#define IDM_FILE_SAVE_AS                           707
#define IDM_FILE_SAVE_ALL                          708
  // the following one is not a true menu item (UI string only)
#define IDM_FILE__EXPORT                           710
#define IDM_FILE__EXPORT_PLAINTEXT                 711
#define IDM_FILE__EXPORT_OLDSTYLEGRF               712
#define IDM_FILE_DOC_PROPERTIES                    713
  // the following one is not a true menu item (UI string only)
#define IDM_FILE_MRU_FILE                          718
#define IDM_FILE_EXIT			                   719

#define IDM_EDIT_OPEN                              735
#define IDM_EDIT_EXTRACT                           736
#define IDM_EDIT_UNDO                              721
#define IDM_EDIT_CUT                               722
#define IDM_EDIT_COPY                              723
#define IDM_EDIT_PASTE                             724
#define IDM_EDIT_RENAME                            725
#define IDM_EDIT_DELETE                            726
#define IDM_EDIT_NEW_FOLDER                        727
#define IDM_EDIT_ADD_FILES                         728
#define IDM_EDIT_ADD_DIRECTORY                     729
#define IDM_EDIT_ADD_FROM_LIST                     730
#define IDM_EDIT_MERGE_WITH_GRF                    731
#define IDM_EDIT_MERGE_FROM_PATCH_LIST             732
#define IDM_EDIT_SELECT_ALL                        733
#define IDM_EDIT_PROPERTIES                        734

#define IDM_VIEW_GRF_EXPLORER_PANE                 741
#define IDM_VIEW_TYPE_TILE                         751
#define IDM_VIEW_TYPE_DETAILS                      752
#define IDM_VIEW_TYPE_LIST                         753
#define IDM_VIEW_TYPE_S_ICONS                      754
#define IDM_VIEW_TYPE_ICONS                        755
  // the following one is not a true menu item (UI string only)
#define IDM_VIEW__SORT                             743
#define IDM_VIEW__SORT_NAME                        744
#define IDM_VIEW__SORT_ORIGIN                      745
#define IDM_VIEW__SORT_EXT                         746
#define IDM_VIEW__SORT_COMPSIZE                    747
#define IDM_VIEW__SORT_EXPSIZE                     748
#define IDM_VIEW_REFRESH                           749


#define IDM_TOOLS_EXTRACT_SPECIFIC                 761
#define IDM_TOOLS_COMPARE                          762
#define IDM_TOOLS_OPTIONS                          776
#define IDM_TOOLS_CUSTOMIZE                        777

#define IDM_WINDOW_NEW                             781
#define IDM_WINDOW_CASCADE                         782
#define IDM_WINDOW_TILE_HORZ                       783
#define IDM_WINDOW_ARRANGE                         784
#define IDM_WINDOW_CLOSE_ALL_BUT_CURRENT           785


#define IDM_HELP_CONTENTS                          801
#define IDM_HELP_FAQ                               802
  // the following one is not a true menu item (UI string only)
#define IDM_HELP__ONLINE                           803
#define IDM_HELP__ONLINE_TUTORIAL                  804
#define IDM_HELP__ONLINE_SUPPORT                   805
#define IDM_FILE__ONLINE_DONATE                    806
#define IDM_HELP_ABOUT			                   810


// UI Strings: menu captions (statusbar / tooltip)
#define IDS_FILE_NEW                   (IDM_FILE_NEW + IDM_IDS_OFFSET)
#define IDS_FILE_NEW_FROM_GRF          (IDM_FILE_NEW_FROM_GRF + IDM_IDS_OFFSET)
#define IDS_FILE_OPEN                  (IDM_FILE_OPEN + IDM_IDS_OFFSET)
#define IDS_FILE_CLOSE                 (IDM_FILE_CLOSE + IDM_IDS_OFFSET)
#define IDS_FILE_CLOSE_ALL_BUT_CURRENT  (IDM_FILE_CLOSE_ALL_BUT_CURRENT + IDM_IDS_OFFSET)
#define IDS_FILE_SAVE                  (IDM_FILE_SAVE + IDM_IDS_OFFSET)
#define IDS_FILE_SAVE_AS               (IDM_FILE_SAVE_AS + IDM_IDS_OFFSET)
#define IDS_FILE_SAVE_ALL              (IDM_FILE_SAVE_ALL + IDM_IDS_OFFSET)

#define IDS_FILE__EXPORT_PLAINTEXT     (IDM_FILE__EXPORT_PLAINTEXT + IDM_IDS_OFFSET)
#define IDS_FILE__EXPORT_OLDSTYLEGRF   (IDM_FILE__EXPORT_OLDSTYLEGRF + IDM_IDS_OFFSET)

#define IDS_FILE_EXIT                  (IDM_FILE_EXIT + IDM_IDS_OFFSET)

#define IDS_EDIT_OPEN                  (IDM_EDIT_OPEN + IDM_IDS_OFFSET)
#define IDS_EDIT_EXTRACT               (IDM_EDIT_EXTRACT + IDM_IDS_OFFSET)
#define IDS_EDIT_UNDO                  (IDM_EDIT_UNDO + IDM_IDS_OFFSET)
#define IDS_EDIT_CUT                   (IDM_EDIT_CUT + IDM_IDS_OFFSET)
#define IDS_EDIT_COPY                  (IDM_EDIT_COPY + IDM_IDS_OFFSET)
#define IDS_EDIT_PASTE                 (IDM_EDIT_PASTE + IDM_IDS_OFFSET)
#define IDS_EDIT_RENAME                (IDM_EDIT_RENAME + IDM_IDS_OFFSET)
#define IDS_EDIT_DELETE                (IDM_EDIT_DELETE + IDM_IDS_OFFSET)
#define IDS_EDIT_NEW_FOLDER            (IDM_EDIT_NEW_FOLDER + IDM_IDS_OFFSET)
#define IDS_EDIT_ADD_FILES             (IDM_EDIT_ADD_FILES + IDM_IDS_OFFSET)
#define IDS_EDIT_ADD_DIRECTORY         (IDM_EDIT_ADD_DIRECTORY + IDM_IDS_OFFSET)
#define IDS_EDIT_ADD_FROM_LIST         (IDM_EDIT_ADD_FROM_LIST + IDM_IDS_OFFSET)
#define IDS_EDIT_MERGE_WITH_GRF        (IDM_EDIT_MERGE_WITH_GRF + IDM_IDS_OFFSET)
#define IDS_EDIT_MERGE_FROM_PATCH_LIST  (IDM_EDIT_MERGE_FROM_PATCH_LIST + IDM_IDS_OFFSET)
#define IDS_EDIT_SELECT_ALL            (IDM_EDIT_SELECT_ALL + IDM_IDS_OFFSET)

#define IDS_VIEW_GRF_EXPLORER_PANE     (IDM_VIEW_GRF_EXPLORER_PANE + IDM_IDS_OFFSET)
#define IDS_VIEW_TYPE_TILE             (IDM_VIEW_TYPE_TILE + IDM_IDS_OFFSET)
#define IDS_VIEW_TYPE_ICONS            (IDM_VIEW_TYPE_ICONS + IDM_IDS_OFFSET)
#define IDS_VIEW_TYPE_LIST             (IDM_VIEW_TYPE_LIST + IDM_IDS_OFFSET)
#define IDS_VIEW_TYPE_DETAILS          (IDM_VIEW_TYPE_DETAILS + IDM_IDS_OFFSET)

#define IDS_VIEW__SORT_NAME            (IDM_VIEW__SORT_NAME + IDM_IDS_OFFSET)
#define IDS_VIEW__SORT_ORIGIN          (IDM_VIEW__SORT_ORIGIN + IDM_IDS_OFFSET)
#define IDS_VIEW__SORT_EXT             (IDM_VIEW__SORT_EXT + IDM_IDS_OFFSET)
#define IDS_VIEW__SORT_COMPSIZE        (IDM_VIEW__SORT_COMPSIZE + IDM_IDS_OFFSET)
#define IDS_VIEW__SORT_EXPSIZE         (IDM_VIEW__SORT_EXPSIZE + IDM_IDS_OFFSET)

#define IDS_VIEW_REFRESH               (IDM_VIEW_REFRESH + IDM_IDS_OFFSET)

#define IDS_TOOLS_EXTRACT_SPECIFIC     (IDM_TOOLS_EXTRACT_SPECIFIC + IDM_IDS_OFFSET)
#define IDS_TOOLS_COMPARE              (IDM_TOOLS_COMPARE + IDM_IDS_OFFSET)
#define IDS_TOOLS_OPTIONS              (IDM_TOOLS_OPTIONS + IDM_IDS_OFFSET)
#define IDS_TOOLS_CUSTOMIZE            (IDM_TOOLS_CUSTOMIZE + IDM_IDS_OFFSET)

#define IDS_WINDOW_NEW                 (IDM_WINDOW_NEW + IDM_IDS_OFFSET)
#define IDS_WINDOW_CLOSE_ALL_BUT_CURRENT  (IDM_WINDOW_CLOSE_ALL_BUT_CURRENT + IDM_IDS_OFFSET)
#define IDS_WINDOW_CASCADE             (IDM_WINDOW_CASCADE + IDM_IDS_OFFSET)
#define IDS_WINDOW_TILE_HORZ           (IDM_WINDOW_TILE_HORZ + IDM_IDS_OFFSET)
#define IDS_WINDOW_ARRANGE             (IDM_WINDOW_ARRANGE + IDM_IDS_OFFSET)

#define IDS_HELP_CONTENTS              (IDM_HELP_CONTENTS + IDM_IDS_OFFSET)
#define IDS_HELP_FAQ                   (IDM_HELP_FAQ + IDM_IDS_OFFSET)

#define IDS_HELP__ONLINE_TUTORIAL      (IDM_HELP__ONLINE_TUTORIAL + IDM_IDS_OFFSET)
#define IDS_HELP__ONLINE_SUPPORT       (IDM_HELP__ONLINE_SUPPORT + IDM_IDS_OFFSET)
#define IDS_FILE__ONLINE_DONATE        (IDM_FILE__ONLINE_DONATE + IDM_IDS_OFFSET)

#define IDS_HELP_ABOUT                 (IDM_HELP_ABOUT + IDM_IDS_OFFSET)






// UI Strings: MDI child frame control titles
#define IDS_APP_TITLE                              500
#define IDS_PANE_TITLE                             502
// UI: Context Menus
#define IDR_LISTVIEW_CTXT                          112
#define IDR_LISTVIEW_ITEM_CTXT                     113
#define IDR_TREEVIEW_CTXT                          114
#define IDR_TREEVIEW_ITEM_CTXT                     115


//   ---   //

// Dialog ID: About Dialog
#define IDD_ABOUTBOX                               201

// Command IDs: About Dialog contextual menu items
#define IDM_REDO                                   600
#define IDM_UNDO                                   601
#define IDM_CUT                                    602
#define IDM_COPY                                   603
#define IDM_PASTE                                  604
#define IDM_DELETE                                 605
#define IDM_SELECTALL                              606
#define IDM_OPEN_URL                               608
#define IDM_COPY_URL                               609

// UI Strings: For l10n (not impl.)
#define IDS_REDO                                   IDM_REDO
#define IDS_UNDO                                   IDM_UNDO
#define IDS_CUT                                    IDM_CUT
#define IDS_COPY                                   IDM_COPY
#define IDS_PASTE                                  IDM_PASTE
#define IDS_DELETE                                 IDM_DELETE
#define IDS_SELECTALL                              IDM_SELECTALL
#define IDS_OPEN_URL                               IDM_OPEN_URL
#define IDS_COPY_URL                               IDM_COPY_URL

// Control ID: About Dialog
#define IDC_ABOUT_CREDITS                          611

// UI Strings: About Dialog
#define IDS_ABOUT_CREDITS                          IDC_ABOUT_CREDITS

//   ---   //

// Dialog ID: Invalid entries Dialog
#define IDD_EXPORT_INVALID_ENTRIES                 202
#define IDC_EDIT_EXPORT_INVALID_ENTRIES_LIST       621


//   ---   //

// Dialog ID: OpenFileName
#define IDD_OFN_ENCODING                           105
#define IDC_OFN_ENABLE_CONV                        631

//   ---   //

// Dialog ID: Progress dialog
#define IDD_PROGRESS                               106
#define IDC_PROGRESS_STATIC                        641
#define IDC_PROGRESS_CAPTION                       642
#define IDC_PROGRESS_CONTROL                       643
#define IDS_PACKING_GRF                            644
#define IDS_PACKING_GRF_ENTRY                      645

//   ---   //

// Dialog ID: Welcome dialog
#define IDD_WELCOME                                107
#define IDC_LICENSE                                651
#define IDR_VIRUS                                  652

//   ---   //

// UI Strings: OpenFileName filters
// Common dialogs filters
#define IDS_FILTER_ALL_FILES                       830
#define IDS_FILTER_GRF_FILES                       831

#define IDS_FILTER_SPR_FILES                       832
#define IDS_FILTER_ACT_FILES                       833
#define IDS_FILTER_ANIM_MULTIFILES                 834

#define IDS_FILTER_RSW_FILES                       835
#define IDS_FILTER_GND_FILES                       836
#define IDS_FILTER_GAT_FILES                       837
#define IDS_FILTER_MAP_MULTIFILES                  838

#define IDS_FILTER_RSM_FILES                       839
#define IDS_FILTER_RSX_FILES                       858
#define IDS_FILTER_GR2_FILES                       840
#define IDS_FILTER_MODELS_MULTIFILES               841

#define IDS_FILTER_TXT_FILES                       842
#define IDS_FILTER_XML_FILES                       843
#define IDS_FILTER_CFG_MULTIFILES                  844

#define IDS_FILTER_BMP_FILES                       845
#define IDS_FILTER_JPG_FILES                       846
#define IDS_FILTER_TGA_FILES                       847
#define IDS_FILTER_STR_FILES                       848
#define IDS_FILTER_PAL_FILES                       849
#define IDS_FILTER_IMF_FILES                       850
#define IDS_FILTER_FNA_FILES                       851
#define IDS_FILTER_GFX_MULTIFILES                  852

#define IDS_FILTER_WAV_FILES                       853
#define IDS_FILTER_MP3_FILES                       854
#define IDS_FILTER_BNK_FILES                       855
#define IDS_FILTER_SND_MULTIFILES                  856

#define IDS_FILTER_ALL_MULTIFILES                  857

// UI Strings: Document library Error Messages
#define IDS_LIBGRF_UNKNOWN_ERROR                   1100
#define IDS_LIBGRF_LIBRARY_ERROR                   1101
#define IDS_LIBGRF_LIBRARY_FAILURE                 1102

// UI Strings: Error Messages
#define IDS_OPENDOC_EMPTY_FAIL                     150
#define IDS_OPENDOC_LOAD_FAIL                      151

// UI Strings: localized elements
#define IDS_NEW_FOLDER_NAME                        870
#define IDS_CANNOT_RENAME_DUPLICATE                871
#define IDS_DELETE_CONFIRMATION_PROMPT             872
#define IDS_DELETE_MULTI_CONFIRMATION_PROMPT       873
#define IDS_MODIFIED_SAVE_PROMPT                   874
#define IDS_CANNOT_CREATE_TEMPFILE                 875
#define IDS_CANNOT_CREATE_DIRECTORY                876
#define IDS_UNTITLED_NAME                          877

#define IDS_MOUNT_LOCATION                         878
#define IDS_USE_CONV_PROMPT                        882


// UI Strings: listview header titles
#define IDS_LVC_NAME                               131
#define IDS_LVC_SIZE                               132
#define IDS_LVC_COMPRESSION                        133
#define IDS_LVC_ORIGIN                             134
#define IDS_LVC_PACKEDSIZE                         135
#define IDS_LVC_TYPE                               136
#define IDS_LVC_PACKRATIO                          137
#define IDS_LVC_RESOURCENAME                       138

// UI Strings: listview column contents
#define IDS_GRFORIGIN_FOLDER                       180
#define IDS_GRFORIGIN_GRF                          181
#define IDS_GRFORIGIN_FS                           182
#define IDS_GRFORIGIN_OTF                          183



#define IDS_SPR_TYPE_DESC                          900
#define IDS_ACT_TYPE_DESC                          901
#define IDS_IMF_TYPE_DESC                          902
#define IDS_FNA_TYPE_DESC                          903
#define IDS_RSW_TYPE_DESC                          904
#define IDS_GND_TYPE_DESC                          905
#define IDS_GAT_TYPE_DESC                          906
#define IDS_RSM_TYPE_DESC                          907
#define IDS_RSX_TYPE_DESC                          919
#define IDS_GR2_TYPE_DESC                          908
#define IDS_TXT_TYPE_DESC                          909
#define IDS_XML_TYPE_DESC                          910
#define IDS_BMP_TYPE_DESC                          911
#define IDS_JPG_TYPE_DESC                          912
#define IDS_TGA_TYPE_DESC                          913
#define IDS_STR_TYPE_DESC                          914
#define IDS_PAL_TYPE_DESC                          915
#define IDS_WAV_TYPE_DESC                          916
#define IDS_MP3_TYPE_DESC                          917
#define IDS_BNK_TYPE_DESC                          918

#define IDS_MENU_FILE                              700
#define IDS_MENU_EDIT                              720
#define IDS_MENU_VIEW                              740
#define IDS_MENU_TOOLS                             760
#define IDS_MENU_WINDOW                            780
#define IDS_MENU_HELP                              800

// Image only (not menu command)
#define IDM_EDIT_ADD_FILES__24                     2728
#define IDM_EDIT_ADD_DIRECTORY__24                 2729
#define IDM_FILE_SAVE__24                          2730



// Window titles
#define IDS_INIT_ERROR_MSGBOX_TITLE                9000
#define IDS_DOC_ERROR_MSGBOX_TITLE                 9001
#define IDS_DELETE_WARNING_MSGBOX_TITLE            9002

#define IDS_INIT_ERROR_COINIT_FAILED               10000
#define IDS_INIT_ERROR_INITCC_FAILED               10001
#define IDS_INIT_ERROR_MODULE_FAILED               10002
#define IDS_INIT_ERROR_IMGLIST_FAILED              10003
#define IDS_INIT_ERROR_MSGLOOP_FAILED              10004
#define IDS_INIT_ERROR_CREATE_MAIN_WINDOW_FAILED   10005
#define IDS_INIT_WARNING_CODEPAGE_NOT_INSTALLED    10006
