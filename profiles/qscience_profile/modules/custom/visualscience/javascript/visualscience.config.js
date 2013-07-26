/*
 * @file 
 * File that manages everything linked with configuration page.
 */
var vsConfig = (function() {

	jQuery(document).ready(function(){
		jQuery('.mini').click(function(clicked) {
			if (jQuery(this).is(':checked')) {
				vsConfig.selectFull(this);
			}
		});
		jQuery('.first, .last').click(function(clicked) {
			vsConfig.selectFull(this);
			vsConfig.selectMini(this);
		});
	});

	return {
		selectMini: function (clicked) {
			jQuery(clicked).parent().siblings().children('.mini').attr('checked', 'checked');
		},
		selectFull: function (clicked) {
			jQuery(clicked).parent().siblings().children('.full').attr('checked', 'checked');
		}

	};

})();
