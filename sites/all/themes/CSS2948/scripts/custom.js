//custom js


jQuery(function(){
            jQuery('select').selectmenu();
    });


jQuery(document).ready(function(){
       
 
    var winWid=jQuery(window).width();
    
  
    if(winWid<481){
    
        jQuery('table').parent('div').css('overflow-x','scroll');
    
    }
    
    
    });