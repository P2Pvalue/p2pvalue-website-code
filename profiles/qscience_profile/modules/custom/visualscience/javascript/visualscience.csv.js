/*
 * @file 
 * File that manages everything linked with downloading users as CSV file.
 */
var vsCSV = (function() {

	return {
		/*
		 * This function exports all the users to a CSV file. Currently using a jQuery plugin.(Table2CSV)
		 */
		exportUsersCSV : function(idOfThisTab) {
			//Some parameters for the communication with the PHP page:
			var newLineCharacter = '[^-|-¼]' + Math.floor(Math.random() * 12);
			var url = vsUtils.getCSVURL();
			var finalTable = '';
			var tableId = 'visualscience-user_list-result-' + idOfThisTab;
			//Through the head of the table
			jQuery('#' + tableId + ' > thead > tr > th').each(function(i) {
				i++;
				if (i != 1) {
					if (i == 2) {
						finalTable += jQuery('#' + tableId + ' > thead > tr > th:nth-child(' + i + ')').text().replace(' ', '-');
					} else {
						finalTable += ',' + jQuery('#' + tableId + ' > thead > tr > th:nth-child(' + i + ')').text().replace(' ', '-');
					}
				}
			});
			finalTable += newLineCharacter;
			//Through the body of table
			jQuery('#' + tableId + ' > tbody > tr').each(function(index) {
				index++;
				if (jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') input').is(':checked')) {
					jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') > td').each(function(cell) {
						cell++;
						if (cell != 1) {
							if (cell == 2) {
								finalTable += jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') > td:nth-child(' + cell + ')').text().replace(' ', '-');
							} else {
								finalTable += ',' + jQuery('#' + tableId + ' > tbody > tr:nth-child(' + index + ') > td:nth-child(' + cell + ')').text().replace(' ', '-');
							}
						}
					});
					finalTable += newLineCharacter;
				}
			});
			finalTable = encodeURIComponent(finalTable) + '&char=' + encodeURIComponent(newLineCharacter);
			window.open(url + finalTable);
		}
	};

})();
