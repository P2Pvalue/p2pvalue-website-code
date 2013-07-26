/*
 * @file 
 * File that manages everything linked with the UI of the app, and how it is managed.
 */

var vsInterface = (function() {

	var tabbedInterfaceExists, tabbedInterface, tabId, createTabbedInterface, listOfViews, overlayModal, nameMaxLength;

    //This variable checks if the whole tabbed interface has been created yet.
    tabbedInterfaceExists = false;

    //Variable who contains the name of the actual tabbed interface.
    tabbedInterface = 'tabbed-interface';

    //Variable to differentiate each tab from each other(0 is for search tab)
    tabId = 1;

    nameMaxLength = 25;

    //List of all the views that have to be pre-loaded
    listOfViews = new Array(
        'actionBar.html',
        'tableUserSearch.html',
        'livingscienceLoading.html',
        'msgRecipientsLayout.html',
        'lsComparisonLayout.html',
        'confRecipientsLayout.html'
        );

    jQuery(document).ready(function() {
        //Pre-loading the views when the html is loaded
        jQuery.each(listOfViews, function(index, element){
            vsInterface.storeViewInDB(element);
        });
        //Creating the dialog div. Check vsInterface.dialog for more infos
        overlayModal = jQuery('<div id="visualscience-overlay-modal"></div>').hide().appendTo('body');
    });

    /*
     * This function creates a tabbed-interface, out of the variable tabbedInterface.
     * Firstly, it however checks if the interface does not already exists, because otherwise this could create bugs.
     */
     createTabbedInterface = function(title, idOfThisTab) {
     	if (!tabbedInterfaceExists) {
     		tabbedInterfaceExists = true;
     		jQuery('#visualscience-container').append('<div id="' + tabbedInterface + '"><ul id="tab-list"></ul></div>');
     		jQuery('#' + tabbedInterface).tabs({
     			cache : true
     		});
     		vsInterface.addTab('<img src="' + vsUtils.getInstallFolder() + 'images/search.png" width="13px" alt="image for visualscience search" /> ', title, '#visualscience-search-tab-content-' + idOfThisTab, true);
     	}
     };

     return {
     	livingscience: false,

     	getTabbedInterface: function (){
     		return tabbedInterface;
     	},

     	getTabId : function() {
     		return tabId;
     	},

     	setTabId : function(newTabId) {
     		tabId = newTabId;
     	},

     	closeDialog: function () {
     		jQuery(overlayModal).dialog("destroy");
     	},

     	dialog: function (content, title, buttons, callback, width, height) {
     		jQuery(overlayModal).dialog('destroy');
     		jQuery(overlayModal).html(content);
     		width = width || 300;
     		height = height || 'auto';
     		var modalButtons;
     		if (buttons) {
     			modalButtons = buttons;
     		}
     		else {
     			modalButtons = [
     			{
     				text: "OK",
     				click: function() {
     					jQuery(overlayModal).dialog("destroy");
     				}
     			}
     			]
     		}
     		jQuery(overlayModal).dialog({
     			modal: true,
     			draggable: false,
     			resizable: false,
     			buttons: modalButtons,
     			title: title,
     			width: width,
     			height: height
     		});
     		if (!title) {
     			jQuery(overlayModal).siblings('.ui-dialog-titlebar').hide();
     		}
     		if (callback) {
     			callback();
     		}
     	},

     	storeViewInDB: function (viewPathSource, callback) {
     		jQuery.get(vsUtils.getInstallFolder() + 'html/' + viewPathSource, function(data) {
     			vsDatabase.htmlViewsDB[viewPathSource] = data;
     			if (callback) {
     				vsInterface.getView(viewPathSource, callback);
     			}
     		});
     	},

     	getView: function (viewPathSource, callback) {
     		if (!vsDatabase.htmlViewsDB[viewPathSource]) {
     			vsInterface.storeViewInDB(viewPathSource, callback);
                 /**
                 * TODO: Here you should find a way to delay the process and then call (because the view is not yet loaded into the db)
                 * return vsInterface.getView(viewPathSource);
                 */
             }
             else {
             	var source = vsDatabase.htmlViewsDB[viewPathSource];
             	source =  Handlebars.compile(source);
             	if (callback) {
             		return callback(source);
             	}
             	else {
             		return source;
             	}
             }
         },

         createTab: function (tabType, idOfThisTab) {
         	switch (tabType) {
         		case 'message':
         		if (vsMessage) {
         			vsMessage.createTabSendMessage(idOfThisTab);
         		}
         		else {
         			vsInterface.dialog('<p>The script file for messages has failed to load. Please try again a bit later.<p>', 'File Error');
         		}
         		break;
         		case 'csv':
         		if (vsCSV) {
         			vsCSV.exportUsersCSV(idOfThisTab);
         		}
         		else {
         			vsInterface.dialog('<p>The script file for CSV exportations has failed to load. Please try again a bit later.<p>', 'File Error');
         		}
         		break;
         		case 'livingscience':
         		if (vsLivingscience && vsInterface.livingscience) {
         			vsLivingscience.createTabLivingScience(idOfThisTab);
         		}
         		else {
         			vsInterface.dialog('<p>The scripts file for LivingScience have failed to load. This is may be due to LivingScience being unreachable. Please try again later.<p>', 'File Error');
         		}
         		break;
         		case 'conference':
         		if (vsConference) {
         			vsConference.createTabConference(idOfThisTab);
         		}
         		else {
         			vsInterface.dialog('<p>The script file for conferences has failed to load. Please try again a bit later.<p>', 'File Error');
         		}
         		break;
         	}
         },

        /*
         * This function is called when the user launches the search from the bar.
         * It will first check if the tabbed interface is loaded and load it if not.
         * Then it adds a new tab to the interface, with the result of the search.
         */
         openUserListTab : function(searchObject) {
         	var title = searchObject.searchQuery ? 'Search: '+ searchObject.searchQuery: 'Search';
         	var idOfThisTab = 0;
         	createTabbedInterface(title, idOfThisTab);
         	var content = vsSearch.createUserSearchResult(searchObject, idOfThisTab);
         	jQuery('#visualscience-search-tab-content-' + idOfThisTab).html(content).css('display', 'block');
         	vsSearch.makeActionBarMoveable(idOfThisTab);
         	vsUtils.makeTableSortable('visualscience-user_list-result-' + idOfThisTab);
         	vsSearch.makeRowsSelectable();
         },

         manageNewSearch: function (searchObject) {
         	var firstTab = jQuery(jQuery(jQuery('#tab-list').children()[0]).children()[0]);
         	firstTab.click();

         	var newTitle = searchObject.searchQuery ? ' Search: '+ searchObject.searchQuery: ' Search';
         	newTitle = newTitle.length > nameMaxLength ? newTitle.substring(0, nameMaxLength) + '... ' : newTitle;
         	var oldTitle = firstTab.text();
         	oldTitle = oldTitle.substring(0, oldTitle.length);
         	var tabTitleContent = firstTab.html().replace(oldTitle, newTitle);
         	firstTab.html(tabTitleContent);
         	var idOfThisTab = 0;
         	var content = vsSearch.createUserSearchResult(searchObject, idOfThisTab);
         	jQuery('#visualscience-search-tab-content-' + idOfThisTab).html(content).css('display', 'block');
         	vsSearch.makeActionBarMoveable(idOfThisTab);
         	vsUtils.makeTableSortable('visualscience-user_list-result-' + idOfThisTab);
         	vsSearch.makeRowsSelectable();
         },

        /*
         * This function adds a new tab to the tabbed interface.
         * The url parameter should be a local url and it can contain a fragment identifier(#something)
         * The name parameter is the name you want the tab to have.
         */
         addTab : function(icon, name, url, closeCross) {
         	if (name.length > nameMaxLength) {
         		name = name.substring(0, nameMaxLength) + '... ';
         	}
         	vsInterface.setTabId(vsInterface.getTabId()+1);
         	var nbTabs = jQuery('#' + tabbedInterface).tabs('length');
         	if (closeCross) {
         		jQuery('#' + tabbedInterface).tabs('add', url, icon + name);
         	}
         	else {
         		jQuery('#' + tabbedInterface).tabs('add', url, icon + name + '<span class="close-tab-cross" onClick="vsInterface.closeTab(\'' + url + '\')">X</span>');
         	}
         	jQuery('#' + tabbedInterface).tabs('select', nbTabs);
         	jQuery('#' + tabbedInterface + ' > .ui-tabs-panel').css({
         		'display' : 'inline-block',
         		'width' : '95.4%',
         		'min-height' : '300px'
         	});
         },
        /*
         * This function closes the tab indicated by tabIndex.
         * TabIndex can either be the zero-position of the tab, or the href parameter.
         */
         closeTab : function(tabIndex) {
         	jQuery('#' + tabbedInterface).tabs('remove', tabIndex);
            //Now we want to delete the database in the array of NDDB
            var tabNb = parseInt(tabIndex.charAt(tabIndex.length - 1));
            vsDatabase.lsDB[tabNb] = undefined;
        }
    };

})();
