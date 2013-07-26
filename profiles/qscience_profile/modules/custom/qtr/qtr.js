jQuery.noConflict();  
jQuery(document).ready(function() {
	jQuery(".tabLink").each(function(){
		jQuery(this).click(function(){
        tabeId = $(this).attr('id');
        jQuery(".tabLink").removeClass("active_tab");
        jQuery(this).addClass("active_tab");
        jQuery(".tabcontent").addClass("hide");
        jQuery("#"+tabeId+"-1").removeClass("hide")   
        return false;	  
      });
    });  
});
