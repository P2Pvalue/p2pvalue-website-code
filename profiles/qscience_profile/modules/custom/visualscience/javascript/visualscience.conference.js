/*
 * @file 
 * File that manages everything linked with the conference tab.
 */

var vsConference = (function() {
	var renameConferenceTab, insertEmailIntoRecipientsDiv, checkForFormErrors, checkAtLeastOneUser;

	checkForFormErrors = function (thisTabId) {
		return vsDatabase.checkConfDate() || vsDatabase.checkConfTitle() || checkAtLeastOneUser(thisTabId);
	};

	checkAtLeastOneUser = function (thisTabId) {
		jQuery('[id^="visualscience-recipients-entry-'+thisTabId+'"]').each(function(index, element) {
			return false;
		});
		return true;
	};

	renameConferenceTab =  function (thisTabId) {
		var nbRecipients = jQuery('#visualscience-recipient-div-content-'+thisTabId+' p').size();
		var title = '';
		if (nbRecipients == 1) {
			title = ' ' + jQuery('#visualscience-recipient-div-content-'+thisTabId+' p a:nth-child(2)').text();
		}
		else if (nbRecipients == 0) {
			title = ' No User';
		}
		else {
			title = ' ' + nbRecipients + ' Users';
		}
		var oldTitle = jQuery('a[href="#conference-tab-' + thisTabId + '"]').text();
		oldTitle = oldTitle.substring(0, oldTitle.length -1);
		var tabTitleContent = jQuery('a[href="#conference-tab-' + thisTabId + '"]').html().replace(oldTitle, title);
		jQuery('a[href="#conference-tab-' + thisTabId + '"]').html(tabTitleContent);
	};

	insertEmailIntoRecipientsDiv = function(thisTabId, email, nbRecipients) {
		nbRecipients += 1;
		vsInterface.getView('confNewRecipientsEntry.html', function(newEntry) {
			var parameters = {
				thisTabId: thisTabId,
				email: email,
				nbRecipients: nbRecipients
			};
			jQuery('#visualscience-recipient-div-content-' + thisTabId).append(newEntry(parameters));
		});
	};

	return {
		/*
		 * Creates a tab for a conference.
		 */
		 createTabConference : function(idOfTheTab) {
		 	selectedUsers = vsSearch.getSelectedUsersFromSearchTable(idOfTheTab);
		 	if (selectedUsers.length > 0) {
		 		var title = vsUtils.getTitleFromUsers(selectedUsers);
		 		var thisTabId = vsInterface.getTabId();
		 		vsInterface.addTab('<img src="' + vsUtils.getInstallFolder() + 'images/conference.png" width="13px" alt="image for conference tab" /> ', title, '#conference-tab-' + thisTabId);

		 		//Create the conference tab
		 		vsInterface.getView('conferenceTabLayout.html', function(confTabView) {
		 			var usersEmail = vsSearch.getSelectedUsersEmailFromSearchTable(idOfTheTab);
		 			var users = new Array();
		 			for (var i=0; i < usersEmail.length; i++) {
		 				users[i] = {
		 					id: i,
		 					name: selectedUsers[i],
		 					email: usersEmail[i],
		 					tab: thisTabId
		 				};
		 			}
		 			var parameters = {
		 				thisTabId: thisTabId,
		 				user: users,
		 				nbUsers: users.length
		 			};
		 			var recipients = vsInterface.getView('confRecipientsLayout.html', function(data) {
		 				return data;
		 			});
		 			parameters.recipients = recipients(parameters);
		 			jQuery('#conference-tab-'+thisTabId).html(confTabView(parameters));
		 			jQuery('.datepicker').datepicker();
		 			vsUtils.loadTimepicker(function(){
		 				jQuery('.timepicker').timeEntry({
		 					show24Hours: true,
		 					spinnerImage: ''

		 				});
		 			});
		 			vsUtils.loadCLEditor('lceEditor'+thisTabId);
		 			vsUtils.loadDrupalHTMLUploadForm('no', 'upload-form-' + thisTabId, thisTabId);
		 		});
} else {
	vsInterface.dialog('Please select at least one user.');
}
},

checkAndSendConference: function (thisTabId) {
	var errors = checkForFormErrors(thisTabId);
	if (!errors) {
		var title = jQuery('#vs-conf-title-'+thisTabId).val();
		var date = jQuery('#vs-conf-date-'+thisTabId).datepicker('getDate').getTime()/1000;
		var start = jQuery('#vs-conf-start-'+thisTabId).timeEntry('getTime');
		var end = jQuery('#vs-conf-end-'+thisTabId).timeEntry('getTime');
		var message = jQuery('#vs-conf-message-' + thisTabId).val();
		var recipients = new Array();
		var attachments = vsUtils.getJsonOfAttachments(thisTabId);
		start = date + start.getHours()*3600 + start.getSeconds();
		end = date + end.getHours()*3600 + end.getSeconds();
		jQuery('p[id*="visualscience-recipients-entry-' + thisTabId + '"]').each(function(i) {
			recipients[i] = new Array(2);
			recipients[i][0] = jQuery(this).children(':nth-child(2)').text();
			recipients[i][1] = jQuery(this).children(':nth-child(2)').attr('href').substring(7);
		});
		//Just make the ajax request now.

	}
	else {
		vsInterface.dialog('Please double check the form as there seems to be some errors.');
	}
},

checkConfTitle: function (thisTabId) {
	if (jQuery('#vs-conf-title-'+thisTabId).val() != '') {
		jQuery('#vs-conf-title-error-'+thisTabId).text('');
		return false;
	}
	else {
		jQuery('#vs-conf-title-error-'+thisTabId).text('You have to set a title.');
		return true;
	}
},

checkConfDate: function (thisTabId) {
	var now = new Date();
	now = Math.floor((now.getTime() - (now.getHours()*3600 + now.getMinutes()*60 + now.getSeconds()))/1000);
	var date = jQuery('#vs-conf-date-'+thisTabId).datepicker('getDate').getTime()/1000;
	if (date >= now && date != null) {
		jQuery('#vs-conf-date-error-'+thisTabId).text('');
		return false;
	}
	else {
		jQuery('#vs-conf-date-error-'+thisTabId).text('The conference should be set after today.');
		return true;
	}
},

addRecipientForConference : function(thisTabId) {
	var email = jQuery('#visualscience-conference-add-recipient-email-' + thisTabId).val();
	if (email.indexOf('@') != -1) {
		var nbRecipients = parseInt(jQuery('#visualscience-conference-add-recipient-button-' + thisTabId).attr('nbRecipients'));
		insertEmailIntoRecipientsDiv(thisTabId, email, nbRecipients);
		jQuery('#visualscience-conference-add-recipient-button-' + thisTabId).attr('nbRecipients', nbRecipients + 1);
		renameConferenceTab(thisTabId);
		jQuery('#visualscience-recipient-div-content-' + thisTabId).scrollTop(jQuery('#visualscience-recipient-div-content-'+thisTabId)[0].scrollHeight);
	} else {
		vsInterface.dialog('Please enter a valid email');
	}
},
deleteRecipientToConference : function(thisTabId, entryNb) {
	jQuery('#visualscience-recipients-entry-' + thisTabId + '-' + entryNb).hide(350, function() {
		jQuery('#visualscience-recipients-entry-' + thisTabId + '-' + entryNb).remove();
		renameConferenceTab(thisTabId);
	});
}
};
})();
