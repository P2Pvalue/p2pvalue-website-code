/*
 * @file 
 * File that manages everything linked with the storing of data.
 */

var vsDatabase = (function() {

	var uploadDB, numberOfPublicationsForLivingScience, firstPublicationForLivingScience, optionsForNDDB, lsDB, lsDBOriginal, db;

	//This variable will store every file that will be uploaded. The first part of the array represent the tab, and the second is the index of the file
	uploadDB = new Array();
	//Constant: The number of publications displayed by default in LS tab
	numberOfPublicationsForLivingScience = 10;

	//Constant: Where to start the display of the LivingScience publications
	firstPublicationForLivingScience = 0;

	//Options for the NDDB database
	optionsForNDDB = {
		tags : {
			'howMany' : numberOfPublicationsForLivingScience,
			'start' : firstPublicationForLivingScience
		}
	};

	return {
		htmlViewsDB : new Array(),

		getUploadDB : function() {
			return uploadDB;
		},
		setUploadDB: function (index, value) {
			uploadDB[index] = value;
		},
		getFirstPublicationForLivingScience : function() {
			return firstPublicationForLivingScience;
		},
		getNumberOfPublicationsForLivingScience : function() {
			return numberOfPublicationsForLivingScience;
		},
		//This is the array containing all the databases result from LivingScience (modified through time by search, display, etc...)
		lsDB : new Array(),
		//The array containing the original result from LS. (as above, but won't be modified)
		lsDBOriginal : new Array(),
		//variable that contain every new NDDB.(the latest created)
		db : new NDDB(optionsForNDDB),

		getOptionsForNDDB : function() {
			return optionsForNDDB;
		},
		setParametersForLSDB : function(thisTabId) {
			jQuery(vsDatabase.lsDB[thisTabId].db).each(function(i) {
				if (vsDatabase.lsDB[thisTabId].db[i].authors && vsDatabase.lsDB[thisTabId].db[i].authors[0] && vsDatabase.lsDB[thisTabId].db[i].authors[0].name) {
					vsDatabase.lsDB[thisTabId].db[i].author = vsDatabase.lsDB[thisTabId].db[i].authors[0].name;
				} else {
					vsDatabase.lsDB[thisTabId].db[i].author = 'Unknown';
					vsDatabase.lsDB[thisTabId].db[i].authors = new Array();
					vsDatabase.lsDB[thisTabId].db[i].authors[0] = {
						name : 'Unknown'
					}
				}
			});
		},
		searchAndSortNDDB : function(thisTabId) {
			var wordToSearch = jQuery('#search-ls-result-' + thisTabId).val().toLowerCase();
			var howMany = vsDatabase.lsDB[thisTabId].resolveTag('howMany');
			var start = vsDatabase.lsDB[thisTabId].resolveTag('start');
			var optionsNDDB = {
				tags : {
					'start' : start,
					'howMany' : howMany
				}
			};
			vsDatabase.lsDB[thisTabId] = new NDDB(optionsNDDB);
			for (var i = 0; i <= vsDatabase.lsDBOriginal[thisTabId].length - 1; i++) {
				var authors = vsDatabase.lsDBOriginal[thisTabId].db[i].author && vsDatabase.lsDBOriginal[thisTabId].db[i].author.toLowerCase().indexOf(wordToSearch) != -1;
				var title = vsDatabase.lsDBOriginal[thisTabId].db[i].title && vsDatabase.lsDBOriginal[thisTabId].db[i].title.toLowerCase().indexOf(wordToSearch) != -1;
				var year = vsDatabase.lsDBOriginal[thisTabId].db[i].year && vsDatabase.lsDBOriginal[thisTabId].db[i].year.toString().toLowerCase().indexOf(wordToSearch) != -1;
				var journal = vsDatabase.lsDBOriginal[thisTabId].db[i].journal && vsDatabase.lsDBOriginal[thisTabId].db[i].journal.toLowerCase().indexOf(wordToSearch) != -1;

				if (authors || title || year || journal) {
					vsDatabase.lsDB[thisTabId].insert(vsDatabase.lsDBOriginal[thisTabId].db[i]);
				}
			}

			var wordResult = 'Result';
			if (vsDatabase.lsDB[thisTabId].length == 0) {
				vsLivingscience.actualizeLivingScienceDisplay(vsDatabase.lsDB[thisTabId], thisTabId);
				jQuery('#ls-list-' + thisTabId).html('<p align="center"><strong>There is no result for your search.</strong></p>');
			} else if (vsDatabase.lsDB[thisTabId].length == 1) {
				vsLivingscience.actualizeLivingScienceDisplay(vsDatabase.lsDB[thisTabId], thisTabId);
			} else {
				vsLivingscience.actualizeLivingScienceDisplay(vsDatabase.lsDB[thisTabId], thisTabId);
				wordResult = 'Results';
			}
			jQuery('#search-ls-nb-result-' + thisTabId).html(vsDatabase.lsDB[thisTabId].length + ' ' + wordResult);
		},
		searchNDDB: function(keyword, database, dbOptions) {
			var searchedDB = new NDDB(dbOptions);
			for (var i = 0; i <= database.length - 1; i++) {
				var authors = database.db[i].author && database.db[i].author.toLowerCase().indexOf(keyword) != -1;
				var title = database.db[i].title && database.db[i].title.toLowerCase().indexOf(keyword) != -1;
				var year = database.db[i].year && database.db[i].year.toString().toLowerCase().indexOf(keyword) != -1;
				var journal = database.db[i].journal && database.db[i].journal.toLowerCase().indexOf(keyword) != -1;

				if (authors || title || year || journal) {
					searchedDB.insert(database.db[i]);
				}
			}
			return searchedDB;
		},
		getNbPublicationsOfLSDB : function(idOfDB) {
			return vsDatabase.lsDBOriginal[idOfDB].count();
		},

		getListJournalsFromLSDB : function(idOfDB) {
			var journalsAll = vsDatabase.lsDBOriginal[idOfDB].fetchArray('journal');
			journalsAll.sort();
			var journals = new Array();
			var html = '<div style="overflow-y:scroll;max-width:250px;max-height:200px;"><ul>';
			jQuery.each(journalsAll, function(i, el) {
				if (jQuery.inArray(el[0], journals) == -1 && el[0] != 'undefined' && el[0] && el[0] != 'NULL') {
					journals.push(el[0]);
					html += '<li>' + journalsAll[i][0] + '</li>';
				}
			});
			html += '</ul></div>';
			var nbJournals = journals.length;
			return '<p><strong>' + nbJournals + ' Journals</strong></p>' + html;
		},
		getListCoauthorsFromLSDB : function(idOfDB) {
			var allAuthors = new Array();
			var authors = new Array();
			authorName = vsLivingscience.getLSTabName(idOfDB);
			allAuthors.push(authorName);
			allAuthors.push(vsUtils.getInitialLastname(authorName));
			allAuthors.push(vsUtils.getLastNameCommaFirstName(authorName));
			allAuthors.push(authorName + '...');
			allAuthors.push(authorName.substring(0, authorName.length - 3));
			var html = '<div style="overflow-y:scroll;max-width:250px;max-height:200px;"><ul>';
			jQuery.each(vsDatabase.lsDBOriginal[idOfDB].db, function(i, el) {
				jQuery.each(el.authors, function(j, element) {
					if (jQuery.inArray(element.name, allAuthors) == -1) {
						allAuthors.push(element.name);
						allAuthors.push(element.name.substring(0, element.name.length - 3));
						allAuthors.push(element.name + '...');
						allAuthors.push(vsUtils.getInitialLastname(element.name));
						allAuthors.push(vsUtils.getLastNameCommaFirstName(element.name));
						authors.push(element.name);
					}
				});
			});
			authors.sort();
			jQuery.each(authors, function(i, el) {
				html += '<li><a href="#" onclick="vsLivingscience.createTabLivingScience(undefined, [\'' + el + '\'])">' + el + '</a></li>';
			});
			html += '</ul></div>';
			var nbOfCoauthors = authors.length;

			return '<p><strong>' + nbOfCoauthors + ' Co-authors</strong></p>' + html;
		},
		getPeriodActivityFromLSDB : function(idOfDB) {
			var min = vsDatabase.lsDBOriginal[idOfDB].min('year');
			var max = vsDatabase.lsDBOriginal[idOfDB].max('year');
			return min + ' - ' + max;
		},
		getFamousPublicationFromLSDB : function(idOfDB) {
			var html = '';
			for ( i = 0; i < 3; i++) {
				html += '<p style="min-width:250px;"><a href="' + vsDatabase.lsDBOriginal[idOfDB].db[i].url + '" target="_blank">' + vsDatabase.lsDBOriginal[idOfDB].db[i].title + '</a></p>';
			}
			return html;
		},
		/*
		 * This function is called when someone selects how to sort the database.
		 * thisTabId is the id of the tab where the database should be sorted, which is also the
		 * index of the database in lsDB.
		 */
		 orderLSResultDatabase : function(thisTabId) {
		 	var orderSetting = jQuery('#sorting-ls-result-' + thisTabId).val();
		 	switch (orderSetting) {
		 		case 'increasing':
		 		vsDatabase.lsDB[thisTabId].sort('year');
		 		break;
		 		case 'decreasing':
		 		vsDatabase.lsDB[thisTabId].sort('year');
		 		vsDatabase.lsDB[thisTabId].reverse();
		 		break;
		 		case 'authors':
		 		vsDatabase.lsDB[thisTabId].sort('author');
		 		break;
		 		case 'random':
		 		vsDatabase.lsDB[thisTabId].shuffle();
		 		break;
		 		case 'own':
		 		vsDatabase.lsDB[thisTabId].sort('livingscienceID');
		 		break;
		 		default:
		 		vsDatabase.lsDB[thisTabId].sort(orderSetting);
		 		break;
		 	}
		 	vsLivingscience.actualizeLivingScienceDisplay(vsDatabase.lsDB[thisTabId], thisTabId);
		 },
		 mergeLSDB : function(firstDatabase, secondDatabase) {
		 	var newDB = new NDDB(optionsForNDDB);
		 	var length = firstDatabase.length > secondDatabase.length ? firstDatabase.length : secondDatabase.length;
		 	var biggerDB = firstDatabase.length > secondDatabase.length ? firstDatabase : secondDatabase;
		 	for (var i = 0; i < length; i++) {
		 		newDB.insert(firstDatabase.db[i]);
		 		newDB.insert(secondDatabase.db[i]);
		 	}
		 	for (var i = length; i < biggerDB.length; i++) {
		 		newDB.insert(biggerDB.db[i]);
		 	}
		 	return newDB;
		 }
		};

	})();
