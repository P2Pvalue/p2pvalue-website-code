/*
 * @file 
 * File that manages everything linked with the Livingscience tab.
 */

var vsLivingscience;
vsLivingscience = (function() {
	var setWidthForMapsAndRelations, livingscience, lslist, lsmap, lsrelations, onLivingScienceResults;
	// We have to wait until the livingscience.nocache.js file is loaded.
	jQuery(window).load(function() {	
		//Object to instatiate the livingscience results (Thanks to this, you will be able to have the ls results) /!\ Needs to be loaded after the file livingscience.nocache.js
		//API instance to make search
		livingscience = new ch.ethz.livingscience.gwtclient.api.LivingScienceSearch();
		//API instance to generate list
		lslist = new ch.ethz.livingscience.gwtclient.api.LivingScienceList();
		//API instance to generate map
		lsmap = new ch.ethz.livingscience.gwtclient.api.LivingScienceMap();
		//API instance to generate relations
		lsrelations = new ch.ethz.livingscience.gwtclient.api.LivingScienceRelations();
		if (livingscience) {
			vsInterface.livingscience = true;
		}
	});

	/*
	 * This function is the callback when a livingscience search is done.
	 * First we get the list of publications, and store them in a NDDB object, which is just a NoSQL database inside JavaScript.
	 * (More infos: https://github.com/nodeGame/NDDB)
	 * Then, thanks to this database, we generate the nice table in the div under the tab.
	 */
	 onLivingScienceResults = function(listOfPublications, idDivUnderTab, thisTabId) {
	 	jQuery('#' + idDivUnderTab).empty();
	 	vsDatabase.db = new NDDB(vsDatabase.getOptionsForNDDB());
	 	vsDatabase.db.importDB(lslist.getPubs(listOfPublications));
	 	vsDatabase.lsDB[thisTabId] = vsDatabase.db;
	 	vsDatabase.lsDBOriginal[thisTabId] = vsDatabase.db;
	 	vsLivingscience.generateLivingScienceFromDB(vsDatabase.lsDB[thisTabId], idDivUnderTab, thisTabId);
	 	jQuery('a[href="#livingscience-tab-' + thisTabId + '"]').bind('click', function() {
	 		vsLivingscience.actualizeLivingScienceDisplay(vsDatabase.lsDB[thisTabId], thisTabId);
	 	});
	 };
	/*
	 * This function sets the layout for the maps and relations div
	 */
	 setWidthForMapsAndRelations = function(listId, mapId, relationsId) {
	 	var setWidth = jQuery('#' + vsInterface.getTabbedInterface()).width() / 2;
	 	setWidth -= setWidth * 1 / 10;
	 	jQuery('#' + mapId + ', #' + relationsId).css({
	 		width : setWidth,
	 		height : setWidth + setWidth * 1 / 10,
	 		display : 'inline-block',
	 		margin : '0px',
	 		padding : '0px'
	 	});
	 	jQuery('#'+listId).css('height', 2 * (setWidth + setWidth * 1 / 10) + 10);
	 };
	 
	 return {
		/*
		 * In this function we create a new LivingScience tab, with the names the end-user checkd in the userlist.
		 * idOfTheTab is the id of the tab where the livingscience request was sent. The optional parameter selectedUsers
		 * is usefull when you already know which are the selected users and is a string separated with ORs (and only ORs).
		 */
		 createTabLivingScience : function(idOfTheTab, selectedUsers) {
		 	if (selectedUsers == undefined) {
		 		selectedUsers = vsSearch.getSelectedUsersFromSearchTable(idOfTheTab);
		 	}
		 	if (selectedUsers.length > 0) {
		 		var title = vsUtils.getTitleFromUsers(selectedUsers);
		 		var thisTabId = vsInterface.getTabId();
		 		vsInterface.addTab('<img src="' + vsUtils.getInstallFolder() + 'images/earth.png" width="13px" alt="image for LivingScience" /> ', title, '#livingscience-tab-' + thisTabId);
		 		livingscience.searchMultipleAuthors(selectedUsers, function(results) {
		 			onLivingScienceResults(results, 'livingscience-tab-' + thisTabId, thisTabId);
		 		});
		 		vsInterface.getView('livingscienceLoading.html', function(loadingPage) {
		 			var inputs = {installFolder: vsUtils.getInstallFolder()};
		 			jQuery('#livingscience-tab-' + thisTabId).html(loadingPage(inputs));
		 		});
		 	} else {
		 		vsInterface.dialog('Please select at least one user');
		 	}
		 },
		/*
		 * Generates the content of a LivingScience tab, with the layout and design.
		 * It firstly creates the layout, and then inserts in it the content.
		 * database is the NDDB database or part of database to send,
		 * location is the div where to insert the content (usually the div of the tab.) and
		 * thisTabId is the id of the tab we are working on, or a unique id for different divs.
		 */
		 generateLivingScienceFromDB : function(database, location, thisTabId) {
		 	var nbResults = database.length;
		 	vsInterface.getView('livingsciencePageLayout.html', function(livingsciencePageContent) {
		 		var parameters = {
		 			totalResults: nbResults,
		 			thisTabId: thisTabId,
		 			firstOrder: Math.floor(nbResults / 12),
		 			secondOrder: Math.floor(nbResults / 6),
		 			thirdOrder: Math.floor(nbResults / 3),
		 			fourthOrder: Math.floor(nbResults / 2),
		 			fifthOrder: Math.floor(nbResults / 1.5),
		 			sixthOrder: Math.floor(nbResults / 1.2),
		 		};
		 		jQuery('#' + location).html(livingsciencePageContent(parameters));
		 		setWidthForMapsAndRelations('ls-list-' + thisTabId, 'ls-map-' + thisTabId, 'ls-relations-' + thisTabId);
		 		vsDatabase.setParametersForLSDB(thisTabId);
		 		vsLivingscience.actualizeLivingScienceDisplay(database, thisTabId);
	 		});
		 },
		/*
		 * Actualizes the display of a LivingScience result.
		 */
		 actualizeLivingScienceDisplay : function(database, thisTabId) {
		 	livingscience = new ch.ethz.livingscience.gwtclient.api.LivingScienceSearch();
		 	lslist = new ch.ethz.livingscience.gwtclient.api.LivingScienceList();
		 	lsrelations = new ch.ethz.livingscience.gwtclient.api.LivingScienceRelations();
		 	lsmap = new ch.ethz.livingscience.gwtclient.api.LivingScienceMap();
		 	var start = database.resolveTag('start');
		 	var howMany = database.resolveTag('howMany');
		 	vsLivingscience.generatePublicationsDiv(database, start, howMany, 'ls-list-' + thisTabId);
		 	vsLivingscience.generateMapDiv(database, start, howMany, 'ls-map-' + thisTabId);
		 	vsLivingscience.generateRelationsDiv(database, start, howMany, 'ls-relations-' + thisTabId);
		 },
		/*
		 * Changes the number of publications displayed in the list, graph and map of a specified tab.
		 * What we do here is that we recreate a new NDDB, with the new wanted parameters, and the
		 * previous NDDB as a base.
		 */
		 changeNumberOfDisplayedLSPublications : function(thisTabId) {
		 	var numberOfPublications = jQuery('#nb-pubs-ls-result-' + thisTabId).val();
		 	if (numberOfPublications == 'all') {
		 		numberOfPublications = vsDatabase.lsDB[thisTabId].length;
		 	}
		 	var options = {
		 		tags : {
		 			'howMany' : numberOfPublications-1,
		 			'start' : vsDatabase.getFirstPublicationForLivingScience()
		 		}
		 	};
		 	vsDatabase.lsDB[thisTabId].init(options, vsDatabase.lsDB[thisTabId]);
		 	vsDatabase.setParametersForLSDB(thisTabId);
		 	vsLivingscience.actualizeLivingScienceDisplay(vsDatabase.lsDB[thisTabId], thisTabId);
		 },
		/*
		 * This function returns the name displayed in a LS tab.
		 */
		 getLSTabName : function(idOfTheTab) {
		 	var tabName = jQuery('a[href|="#livingscience-tab-' + idOfTheTab + '"]').text();
		 	tabName = tabName.substring(1, tabName.length - 1);
		 	return tabName;
		 },
		/*
		 * Generates the design of the LS list of publications and puts it into an already existing div.
		 * database is the NDDB data,
		 * start is which entry we want to display first(usually 0),
		 * howMany is the number of entries to display,
		 * and location is where to insert the content once it is created (without #)
		 */
		 generatePublicationsDiv : function(database, start, howMany, location) {
		 	var publicationsToShow = new Array();
		 	for (var i = start; (i <= start + howMany) && (i <= database.db.length - 1); i++) {
		 		publicationsToShow.push(database.db[i].livingscienceID);
		 	}
		 	lslist.generateList(publicationsToShow, location);
		 },

		/*
		 * Generates the design of the LS Relations graph and puts it into an already existing div.
		 * database is the NDDB data,
		 * location is the id without # of where to insert it
		 */
		 generateRelationsDiv : function(database, start, howMany, location) {
		 	var publicationsToShow = new Array();
		 	for (var i = start; (i <= start + howMany) && (i <= database.length - 1); i++) {
		 		publicationsToShow.push(database.db[i].livingscienceID);
		 	}
		 	lsrelations.set(publicationsToShow, location);
		 },

		/*
		 * Generates the design of the LS Map graph and puts it into an already existing div.
		 * database is the NDDB data,
		 * location is the id without # of where to insert it
		 */
		 generateMapDiv : function(database, start, howMany, location) {
		 	var publicationsToShow = new Array();
		 	for (var i = start; (i <= start + howMany) && (i <= database.length - 1); i++) {
		 		publicationsToShow.push(database.db[i].livingscienceID);
		 	}
		 	lsmap.set(publicationsToShow, location);
		 }
		};

	})();