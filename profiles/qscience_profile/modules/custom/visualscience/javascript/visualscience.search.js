/*
 * @file 
 * File that manages everything linked with the display of users in a table.
 *
 * Note: For searching users functionnalities, you have to check userlist.js
 */
var vsSearch = (function() {

	return {
		makeRowsSelectable : function() {
			jQuery('.clickToSelect').click(function() {
				var cur = jQuery(this).children().children().children();
				var newState = !(cur.attr('checked'));
				cur.attr('checked', newState);
				jQuery(this).toggleClass('vsSelectedRow');
			});
			jQuery('.clickToSelect').children().children().children().click(function() {
				var newState = !(jQuery(this).attr('checked'));
				jQuery(this).attr('checked', newState);
				jQuery(this).toggleClass('vsSelectedRow');
			});
		},

		/*
		 * This function creates the whole tab, which will be displayed to the user.
		 * It contains :
		 * -the action bar, which is the bar with every buttons(Message, CSV, LS and Conference)
		 * -The table with the result and its options.(Sort table, hide fields, etc...)
		 */
		 createUserSearchResult : function(searchObject, idOfThisTab) {
		 	var actionBar = vsSearch.createActionBar(idOfThisTab);
		 	var tableUserList = vsSearch.createTableUserList(searchObject, idOfThisTab);
		 	return '<h3>User List</h3>' + actionBar + tableUserList;
		 },

		/*
		 * This creates the action bar, with the different buttons.
		 */
		 createActionBar : function(idOfThisTab) {
		 	var actionBar = vsInterface.getView('actionBar.html', function(data) {
		 		return data;
		 	});
		 	var parameters = {
		 		idOfThisTab: idOfThisTab,
		 		installFolder: vsUtils.getInstallFolder()
		 	};
		 	return actionBar(parameters);
		 },
		/*
		 * Depending on what the user sees, the action bar will be static at the top of the page,
		 * or fixed on the left, when he scrolls down.
		 */
		 makeActionBarMoveable : function(idOfThisTab) {
		 	var top_offset = jQuery('#action-bar-container' + idOfThisTab).offset().top;
		 	var tableHeight = jQuery('#visualscience-user_list-result-' + idOfThisTab).height();
		 	var actionBarHeight = jQuery('#actionBar' + idOfThisTab).height();
		 	if (tableHeight > actionBarHeight) {
		 		jQuery('#action-bar-container' + idOfThisTab).height(tableHeight);
		 	}
		 	var el = jQuery('#actionBar' + idOfThisTab);
		 	jQuery(window).bind('scroll', function() {
		 		var scroll_top = jQuery(window).scrollTop();
		 		var threshold = 100;
				//a threshold so the bar does not stick to the top
				var tabHeight = jQuery('#visualscience-search-tab-content-' + idOfThisTab).height();
				if (scroll_top + threshold + actionBarHeight > top_offset + tableHeight && tabHeight > 350) {
					el.css('top', tableHeight - actionBarHeight);
				} else if (scroll_top > top_offset - threshold) {
					el.css('top', scroll_top - top_offset + threshold);
				} else {
					el.css('top', '');
				}
			});
		 },
		/*
		 * This function gets every selected user from the user-list of results.
		 * It returns an array with the full name of each users.
		 */
		 getSelectedUsersFromSearchTable : function(idOfTheTab) {
		 	var tableId = 'visualscience-user_list-result-' + idOfTheTab;
		 	var completeNamesArray = new Array();
		 	var firsts = new Array();
		 	var lasts = new Array();
		 	if (jQuery('#'+tableId+' .visualscience-search-field-last').length > 0) {
		 		var nbLastRow = vsUtils.getRowNbWithClass(tableId, 'visualscience-search-field-last');
		 		jQuery('#' + tableId + ' > tbody > tr').each(function(index) {
		 			index++;
		 			if (jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') input').is(':checked')) {
		 				var last = jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') > td:nth-child(' + (nbLastRow+1) + ')').text();
		 				last = vsUtils.stripSpacesStartEnd(last);
		 				lasts.push(last);
		 			}
		 		});
		 	}
		 	//firsts, to do anyway
		 	var nbFirstRow = vsUtils.getRowNbWithClass(tableId, 'visualscience-search-field-first');
		 	jQuery('#' + tableId + ' > tbody > tr').each(function(index) {
		 		index++;
		 		if (jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') input').is(':checked')) {
		 			var first = jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') > td:nth-child(' + (nbFirstRow+1) + ')').text();
		 			first = vsUtils.stripSpacesStartEnd(first);
		 			firsts.push(first);
		 		}
		 	});
		 	vsUtils.stripSpacesStartEnd();
		 	//merging both
		 	if (lasts.length > 0) {
		 		jQuery.each(lasts, function(i, el) {
		 			completeNamesArray.push(firsts[i]+' '+lasts[i]);
		 		});
		 	}
		 	else {
		 		completeNamesArray = firsts
		 	}

		 	return completeNamesArray;
		 },
		/*
		 * This function gets every selected user's email from the user-list of results.
		 * It returns an array with the email of each users.
		 */
		 getSelectedUsersEmailFromSearchTable : function(idOfTheTab) {
		 	var tableId = 'visualscience-user_list-result-' + idOfTheTab;
		 	var firstFieldNumber = vsUtils.getThWithContent(tableId, 'Mail');
		 	if (firstFieldNumber == -1) {
		 		vsInterface.dialog('The email field has to be enabled to use this function.');
		 		return false;
		 	}
		 	var emailArray = new Array();
		 	jQuery('#' + tableId + ' > tbody > tr').each(function(index) {
		 		index++;
				//That's because index will go from 0(no nth-child) to n-1, missing n (interesting)
				if (jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') input').is(':checked')) {
					emailArray.push(jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') > td:nth-child(' + firstFieldNumber + ')').text());
				}
			});
		 	return emailArray;
		 },

		/*
		 * Creates the table of users, which can be sorted.
		 */
		 createTableUserList : function(searchObject, idOfThisTab) {
		 	var searchTable = vsInterface.getView('tableUserSearch.html');
		 	var parameters = {
		 		header: searchObject.fields,
		 		tabId: idOfThisTab,
		 		users: searchObject.users,
		 		nbEntries: searchObject.limit
		 	};
		 	var divFinalContent = searchTable(parameters);
		 	divFinalContent += vsSearch.getTableUserListOptions(searchObject.fields, idOfThisTab);
		 	return divFinalContent;
		 },
		/*
		 * This function creates the visibility options for the user list search.
		 * firstly it takes every th field from the header table, and generates the checkbox witht these labels.
		 * On the checkbox there is a function that toggles the visibility of the wanted element.
		 */
		 getTableUserListOptions : function(fields, idOfThisTab) {
		 	var divOptions = '<fieldset class="collapsible form-wrapper" id="edit-fields"><legend><span class="fieldset-legend"><a onClick="jQuery(\'#edit-fields > .fieldset-wrapper\').slideToggle();">Choose fields to show</a></span></legend><div class="fieldset-wrapper" style="display:none;"><div style="max-height: 300px; overflow: auto">';
		 	jQuery.each(fields, function(i, el) {
		 		el = el.replace(/<(?:.|\n)*?>/gm, '');
		 		if (el != '') {
		 			divOptions += '<div class="form-item form-type-checkbox form-item-user-data-name" style="width:50%; display:inline-block;"><label for="checkbox-visibility-' + el + idOfThisTab + '" class="option"><input type="checkbox" onClick="vsSearch.toggleColNbFromTable(\'visualscience-user_list-result-' + idOfThisTab + '\',' + (i + 1) + ');" checked="checked" class="form-checkbox" name="checkbox-visibility-' + el + idOfThisTab + '" id="checkbox-visibility-' + el + idOfThisTab + '" /> ' + el + ' </label></div>';
		 		}
		 	});
		 	divOptions += '</div></div></fieldset>';
		 	return divOptions;
		 },
		/*
		 * This function selects all checkboxes once you click on the top
		 * checkbox of a user-list search table. It firstly checks if the
		 * top box is checked or not, and then apply the state to all the boxes.
		 */
		 selectAllBoxes : function(idOfThisTab) {
		 	var newState;
		 	if (jQuery('#user-list_master_checkbox-' + idOfThisTab).attr('checked') == true) {
		 		newState = false;
		 	} else {
		 		newState = true;
		 	}
		 	jQuery('#user-list_master_checkbox-' + idOfThisTab).attr('checked', newState);
		 	jQuery('#visualscience-user_list-result-' + idOfThisTab + ' input[id|="user_list-list"]').each(function() {
		 		jQuery(this).attr('checked', newState);
		 	});
		 },
		/*
		 * toggles the visibility of a column in a table.
		 * tableId is the id of the table and colNb the number (from 0)  of the col to toggle.
		 */
		 toggleColNbFromTable : function(tableId, colNb) {
		 	jQuery('#' + tableId + ' td:nth-child(' + (colNb + 1) + ')').toggle();
		 	jQuery('#' + tableId + ' th:nth-child(' + (colNb + 1) + ')').toggle();
		 },
		};

	})();
