/*
 * @file 
 * File that provides some usefull tools to other files.
 */

var vsUtils = (function() {
    var urlPath, rootFolder, installFolder, UploadModuleURL, SendMailURL, csvURL, usersPath;
    jQuery(document).ready(function() {
        //This is the root folder, where the installation has been done.
        rootFolder = 'http://' + window.location.hostname + '' + Drupal.settings.basePath;
        //This is the folder in which visualscience is installed (Should be already defined thanks to PHP.)
        installFolder = Drupal.settings.installFolder;//'sites/all/modules/visualscience/';

        urlPath = (function () {
            var detect = 'visualscience';
            var idx = -1;
            idx = unescape(location.href).lastIndexOf(detect);
            return unescape(location.href).substring(0,idx + detect.length);
        })();

        usersPath = urlPath + '/users';
        //This is the URL to the php upload module
        UploadModuleURL = rootFolder + 'visualscience/upload/';
        //This is the URL to the php that handles the mail
        SendMailURL = rootFolder + 'visualscience/mail/';
        csvURL = installFolder + 'includes/stringToCSV.php?text=';
        //Testing wether the browser is IE6-8:
        if (jQuery.browser.mise && parseInt(jQuery.browser.version.slice(0, 3)) < 9) {
            jQuery.getScript(installFolder + '/javascript/lib/visualscience.shim.IE9.js');
        }
    });
    //This is the DialogNumber variable. Setting it global makes everything much more easier to use.
    var dialogNumber;

    return {
    	getUrlPath: function ()  {
    		return urlPath;
    	},
    	getUsersPath: function () {
    		return usersPath;
    	},
    	getRootFolder: function () {
    		return rootFolder;
    	},
    	getInstallFolder: function() {
    		return installFolder;
    	},
    	getCSVURL : function() {
    		return csvURL;
    	},
    	getSendMailURL : function() {
    		return SendMailURL;
    	},

    	isLoggedIn: function() {
    		return !!(Drupal.settings.username);
    	},

        /*
         * Automatically turns a table into a sortable table.(jQuery Plugin: Tablesorter 2.0)
         * the parameter idOfTable is the actual id of the table to be sorted.
         * Warning: The first column won't be sortable.(That's the reason for parameter headers:{0...})
         */
         makeTableSortable : function(idOfTable) {
         	jQuery('#' + idOfTable).tablesorter({
         		headers : {
         			0 : {
         				sorter : false
         			}
         		}
         	});
         },

         loadUploadScripts : function(areaId, callback) {
         	jQuery.getScript(installFolder + '/javascript/lib/visualscience.jquery.form.js');
         },

         loadTimepicker: function(callback) {
         	jQuery.getScript(installFolder+'/javascript/lib/visualscience.jquery.timeentry.js', callback);
         },

         uploadSubmittedFiles : function(tabId) {
         	var nbFilesEntered = parseInt(jQuery('#upload-form-' + tabId + ' #edit-visualscience-upload-file').attr('nbFiles'));
         	var fileList = jQuery('#upload-form-'+tabId+' #edit-visualscience-upload-file')[0];
         	var content = '';
         	if (!vsDatabase.getUploadDB()[tabId]) {
         		vsDatabase.setUploadDB(tabId, new Array());
                jQuery('#upload-form-' + tabId + ' #visualscience-upload-form').ajaxForm({
                    clearForm : true,
                    beforeSend : function() {
                     jQuery('#progress-upload-' + tabId).text('Progress: Preparing File...').css({
                      'background-color' : 'yellow',
                      'display' : 'block',
                      'color' : 'white'
                  });
                 },
                 uploadProgress : function() {
                     jQuery('#progress-upload-' + tabId).text('Progress: Sending File... Please Wait.').css('background-color', 'orange');
                 },
                 success : function(data, textStatus, jqXHR) {
                    var invisible = data.substring(data.indexOf('</head>'));
                    jQuery('html').append('<div id="invisible" style="display:none;">' + invisible + '</div>');
                    jQuery('#invisible').html(jQuery('#invisible .messages').html());
                    var messages = jQuery('#invisible').text();
                        if (messages.indexOf('Error') != -1) {//Check for errors
                        	jQuery('#progress-upload-' + tabId).text('Upload Failed: ' + messages).css({
                        		'background-color' : 'red'
                        	});
                        } else if (messages.indexOf('Status') != -1) {//check for success and path
                        	var nbFilesEntered = parseInt(jQuery('#upload-form-' + tabId + ' #edit-visualscience-upload-file').attr('nbFiles'));
                        	var link = messages.substring(messages.indexOf('The file has been uploaded to:') + 30);
                        	var fileName = jQuery('#upload-form-' + tabId + ' #edit-visualscience-upload-file').val().replace('c:\\fakepath\\', '').replace('C:\\fakepath\\', '');
                        	var newLine = '<p id="visualscience-upload-file-entry-' + tabId + '-' + (nbFilesEntered + 1) + '" class="entryFile"><a onMouseOut="jQuery(this).css(\'color\', \'\');" onMouseOver="jQuery(this).css({\'color\': \'#FF0000\', \'text-decoration\':\'none\'});" onClick="vsUtils.deleteFileToUpload(' + tabId + ', ' + (nbFilesEntered + 1) + ');" id="visualscience-message-close-cross-' + tabId + '-' + (nbFilesEntered + 1) + '" style="border-right:solid black 1px;font-size:20px;padding-right:15px;padding-left:15px;margin-right:20px;">X</a><a class="visualscience-upload-file-entry-name" href="' + link + '" target="_blank">' + fileName + '</a></p>';
                        	jQuery('#visualscience-message-attachments-div-show-' + tabId).append(newLine);
                        	jQuery('#upload-form-' + tabId + ' #edit-visualscience-upload-file').attr('nbFiles', nbFilesEntered + 1)
                        	jQuery('#visualscience-message-attachments-div-show-' + tabId).scrollTop(jQuery('#visualscience-message-attachments-div-show-'+tabId)[0].scrollHeight);
                        	jQuery('#progress-upload-' + tabId).text('File Successfully Uploaded ! You may select another one.').css({
                        		'background-color' : 'green'
                        	});
                        } else {//Improbable, there was an error.
                        	jQuery('#progress-upload-' + tabId).text('Progress: Upload Unsuccessful. Please Try Again.').css({
                        		'background-color' : 'red'
                        	});
                        }
                        jQuery('#invisible').remove();
                    },
                    error : function(jqXHR, textStatus, errorThrown) {
                    	jQuery('#progress-upload-' + tabId).text('Progress: Error ' + textStatus + ': ' + errorThrown).css({
                    		'background-color' : 'red'
                    	});
                    }
                });
}
jQuery('#upload-form-' + tabId + ' #visualscience-upload-form').submit();
return false;

},
deleteFileToUpload : function(tabId, entryNb) {
	jQuery('#visualscience-upload-file-entry-' + tabId + '-' + entryNb).hide(350, function() {
		jQuery('#visualscience-upload-file-entry-' + tabId + '-' + entryNb).remove();
	});
},

loadCLEditor : function(areaId) {
	if (document.createStyleSheet) {
		document.createStyleSheet(installFolder + '/css/visualscience.jquery.cleditor.css');
	} else {
		jQuery("head").append(jQuery("<link rel='stylesheet' href='" + installFolder + "/css/visualscience.jquery.cleditor.css' type='text/css' media='screen' />"));
	}
	jQuery.getScript(installFolder + '/javascript/lib/visualscience.jquery.cleditor.min.js', function() {
		jQuery('#' + areaId).cleditor({
			width : '100%',
			height : '440px'
		});
	});
},
        /*
         * Modifies a Drupal-generated form into a visually more estheatical form.
         */
         loadDrupalHTMLUploadForm : function(html, location, thisTabId) {
         	jQuery('#' + location).load(UploadModuleURL + ' #visualscience-upload-form', function(response, status, xhr) {
         		if (status == "error") {
         			if (xhr.status == 403) {
         				vsInterface.dialog('Please login to be able to send messages.(403)');
         				jQuery('#' + location).html('<p align="center" font-color="red">Please login to be able to send messages.</p>');
         			} else if (xhr.status == 404) {
         				vsInterface.dialog('Please come back later, there is a problem with the server.(404)');
         			} else {
         				vsInterface.dialog('An error occured:\n' + 'Status:' + xhr.status + ':\n' + xhr.statusText);
         			}
         		} else {
         			jQuery('#' + location + ' #edit-submit').hide();
         			jQuery('#' + location + ' #edit-visualscience-upload-file').attr({
         				'onChange' : 'vsUtils.uploadSubmittedFiles(\'' + thisTabId + '\');',
         				'nbFiles' : '0',
         				'size' : '18'
         			}).css({
         				'width' : '350px',
         				'margin-left' : '15px',
         				'display' : 'block'
         			});
         		}
         	});
},

getJsonOfAttachments : function(thisTabId) {
	var attachments = new Array();
	jQuery('p[id*="visualscience-upload-file-entry-' + thisTabId + '"]').each(function(i) {
		attachments[i] = new Array(2);
                //Name of File:
                attachments[i][0] = jQuery(this).children(':nth-child(2)').text();
                //URL of File:
                attachments[i][1] = jQuery(this).children(':nth-child(2)').attr('href');
            });
	return attachments;
            //window.JSON.stringify(attachments);
        },
        changeArrayToOrString : function(selectedUsers) {
        	var string = '"';
        	for (var i = 0; i < selectedUsers.length - 1; i++) {
        		string += selectedUsers[i] + '" OR "';
        	}
        	string += selectedUsers[selectedUsers.length - 1] + '"';
        	return string;
        },
        /*
         * This functions sets the title for tabs, depending on the selected users.
         */
         getTitleFromUsers : function(selectedUsers) {
         	var nbUsers = selectedUsers.length;
         	title = (nbUsers > 1 ? nbUsers + ' Users' : selectedUsers[0]);
         	return title;
         },
        /*
         * With this function, you get the column number from a table, whose column's th contains fieldContent.
         * tableId is the id of the table you want to check for the column number.
         * fieldContent is the content of the th the column should have.
         */
         getThWithContent : function(tableId, fieldContent) {
         	for (var i = 0; i <= vsUtils.countColumnsInTable(tableId); i++) {
         		var thTitle = jQuery('#' + tableId + ' > thead > tr > th:nth-child(' + i + ')').text();
         		thTitle = vsUtils.stripSpacesStartEnd(thTitle);
         		if (thTitle == fieldContent) {
         			return i;
         		}
         	}
         	return -1;
         },
         getLastNameCommaFirstName : function(name) {
         	var first = name.substring(0, name.lastIndexOf(' '));
         	var last = name.substring(name.lastIndexOf(' ') + 1);
         	return last + ', ' + first;
         },
         getInitialLastname : function(name) {
         	var initial = name.substring(0, 1);
         	var last = name.substring(name.lastIndexOf(' ') + 1);
         	return initial + ' ' + last;
         },
         sortHTMLList : function(idOfList) {
         	var mylist = jQuery('#' + idOfList);
         	var listitems = mylist.children('li').get();
         	listitems.sort(function(a, b) {
         		return jQuery(a).text().toUpperCase().localeCompare(jQuery(b).text().toUpperCase());
         	})
         	jQuery.each(listitems, function(idx, itm) {
         		mylist.append(itm);
         	});
         },

        /*
         * Count and returns the number of columns in a table.
         * tableId is the id of the table.
         */
         countColumnsInTable : function(tableId) {
         	var colCount = 0;
         	jQuery('#' + tableId + ' > thead > tr > th').each(function() {
         		colCount++;
         	});
         	return colCount;
         },
        /*
         * Transform the content of a table into an array. It doesn't take in account the tr,
         * so it only returns the value of the table, without any hierarchic order.(NO array in array)
         */
         getArrayFromTable : function(tableId) {
         	var values = new Array();
         	var td = jQuery('#' + tableId + ' > tbody > tr > td');
         	td.each(function(i) {
         		values[i] = jQuery(this).html();
         	})
         	return values;
         },

         getRowNbWithClass: function (tableId, classToFind) {
         	var result = 0;
         	jQuery('#'+tableId+' th').each(function(i, el) {
         		if (jQuery(el).children().length > 0 && jQuery(el).children().hasClass(classToFind)) {
         			result = i;
         		}
         	});
         	return result;
         },

         stripSpacesStartEnd: function (str) {
         	if (str) {
         		return str.replace(/^\s\s*/, '').replace(/\s\s*$/, '');
         	}
         	return str+'';
         }

     };

 })();
