var i = 0;
//console.log('qscience.js imported.');
var events = null;

(function ($) {
  
//  $('input[name="author_reference_add_more"]').live('mouseup', function () {
//    
//  });
  
//    Drupal.behaviors.qscience = {
//      attach: function (context, settings) {
//        $('#edit-author-reference-und-add-more').once('del-default-events', function() {
//          console.log('unbinding...');
//          $('#edit-author-reference-und-add-more', context).unbind('mousedown');
//        });
//        console.log('attach running');
//        var e = $('#edit-author-reference-und-add-more').data('events');
//        events = new Array();
//        events = $.extend(true, [], e['mousedown']);
//        $('#edit-author-reference-und-add-more', context).unbind('mousedown');
//        $('#edit-author-reference-und-add-more').click(function() {
//          console.log('click');
//          if (Math.random() > 0.8) {
//            console.log('20%');
//            for (var ev in events) {
//              e = events[ev].handler;
//              console.log('calling saved event...');
//              e();
//            }
//          } else {
//            console.log('80%');
//          }
//        });
//      }
//    };
  
//  Drupal.behaviors.qscience = {
//    attach: function (context, settings) {
////      i = 0;
//      if (typeof context == 'object' && typeof context[0] != 'undefined') {
//        var id = context.attr('id');
//        if (id == 'paper-node-form') {
//          // Save and remove default handlers anyway...
//          var test;
//          var e = $('#edit-author-reference-und-add-more').data('events');
//          events = new Array();
//          test = events = $.extend(true, [], e['mousedown']);
////          console.log(events.length);
//          $('#edit-author-reference-und-add-more', context).unbind('mousedown');
//          e = $('#edit-author-reference-und-add-more').data('events');
//          test = $.extend(true, [], e['mousedown']);
//          console.log(events);
//          console.log(test);
//          
//          var rownum = $('#author-reference-values tr.draggable').length;
//          console.log('rownum -> ' + rownum);
//          
//          i++;
//          console.log(i);
//          if ((rownum == 2) || (rownum == 3 && i == 1) || (i + 3 === rownum)) {
//            console.log(i + ' == ' + rownum);
////            i = 0;
//            $('input[name="author_reference_add_more"]', context).live('mouseup', function () {
//  //            console.log('mouseup event....');
//              var showHidden = false;
//              $('#author-reference-values tr.draggable').each(function() {
//  //              console.log('class -> ' + $(this).attr('class'));
//                if ($(this).hasClass('hidden')) {
//                  if (showHidden) return;
//                  showHidden = true;
//                  $(this).removeClass('hidden');
//                }
//              });
//              console.log('show hidden: ' + showHidden);
//              if (!showHidden) {
//                for (var ev in events) {
//                  e = events[ev].handler;
//  //                console.log('calling saved event...');
//                  e();
//                }
//              }
//              return false;
//            });
//          } else {
//            console.log('skip validation...');
//            return false;
//          }
//        }
//      }
//      
////      console.log(settings);
//      console.log('- - - - -');
////      console.log($('input[name="author_reference_add_more"]').length);
//
//    }
//  };
  
  $(document).ready(function() {
//    console.log('document.ready');
//    var e = $('#edit-author-reference-und-add-more').data('events');
//    events = $.extend(true, [], e['mousedown']);
       
//    $('input.author-remove').live('click', function() {
//      $(this).closest('tr.draggable').addClass('hidden');
//      console.log('hide it!');
//      return false;
//    });
    
//    console.log(events);
//    $('#edit-author-reference-und-add-more').unbind('mousedown');
//    $('#edit-author-reference-und-add-more').live('mouseup', function() {
//      console.log('mouseup event....');
//      var showHidden = false;
//      $('#author-reference-values tr.draggable').each(function() {
//        console.log($(this).attr('class'));
//        if ($(this).hasClass('hidden')) {
//          if (showHidden) return;
//          showHidden = true;
//          $(this).removeClass('hidden');
//        }
//      });
//      if (!showHidden) {
//        for (var ev in events) {
//          var e = events[ev].handler;
//          e();
//        }
//      }
//      return false;
//    });
  });
})(jQuery);
  

////(function ($) {
//  $(document).ajaxComplete(function(e, xhr, settings) {
//    if (settings.extraData._triggering_element_name == 'field_paper_upload_und_0_upload_button') {
//      var num = $('#edit-field-paper-upload div.messages').length;
//      var i = 1;
//      $('#edit-field-paper-upload div.messages').each(function() {
//        if (i >= num) return;
//        $(this).slideUp(1000);
//        i++;
//      });
//      if (Drupal.settings && Drupal.settings.pdfparser) {
//        if (Drupal.settings.pdfparser.title) {
//          $('#edit-title').val(Drupal.settings.pdfparser.title);
//        }
//        if (Drupal.settings.pdfparser.abstr) {
//          $('#edit-abstract-und-0-value').val(Drupal.settings.pdfparser.abstr);
//        }
//        if (Drupal.settings.pdfparser.authors) {
//          for (var i = 0; i < Drupal.settings.pdfparser.authors.length; i++) {
//            addAuthor(i+1, Drupal.settings.pdfparser.authors[i]);
//          }
//        }
//        //console.log(Drupal.settings.pdfparser.citations);
//        if (Drupal.settings.pdfparser.citations) {
//          for (var i = 0; i < Drupal.settings.pdfparser.citations.length; i++) {
//            var title = Drupal.settings.pdfparser.citations[i].title;
//            if (title.length > 0) {
//              addCitation(i+1, title);
//            }
//          }
//        }
//      }
//    } else if (settings.extraData._triggering_element_name == 'field_paper_upload_und_0_remove_button') {
//      $('#edit-title').val('');
//      $('#edit-abstract-und-0-value').val('');
//      $('#edit-authorname-und-0-value').val('');
//      $('#edit-field-paper-upload div.messages').slideUp(400);
//      var i = 2;
//      while (jQuery('div.form-item-author-'+i).length != 0) {
//        jQuery('div.form-item-author-'+i).remove();
//        i++;
//      }
//      jQuery('div.form-item-author-1 input').val('');
//      i = 2;
//      while (jQuery('div.form-item-citation-'+i).length != 0) {
//        jQuery('div.form-item-citation-'+i).remove();
//        i++;
//      }
//      jQuery('div.form-item-citation-1 input').val('');
//
//    }
//  });
//})(jQuery);
//
//function addAuthor(i, name) {
//  var num = 1;
//  while (true) {
//    if (jQuery('div.form-item-author-'+num).length == 0) {
//      break;
//    } else {
//      num++;
//    }
//  }
//  if (typeof i != 'undefined' && typeof name != 'undefined') {
//    if (i >= num) {
//      makeAuthorCopy(num, name);
//    } else {
//      jQuery('div.form-item-author-'+i+' input').val(name);
//    }
//  } else {
//    makeAuthorCopy(num);
//  }
//
//  return false;
//}
//
//function makeAuthorCopy(num, name) {
//  var copy = jQuery('div.form-item-author-1').clone();
//  if (typeof name != 'undefined') {
//    copy.find('input').val(name);
//  } else {
//    copy.find('input').val('');
//  }
//  copy.removeClass('form-item-author-1').addClass('form-item-author-'+num);
//  copy.find('label').attr('for', 'edit-author-'+num);
//  copy.find('#edit-author-1').attr('id', 'edit-author-'+num).attr('name', 'author_'+num);
//
//  jQuery('#edit-author-adder').before(copy);
//}
//
//function addCitation(i, name) {
//  var num = 1;
//  while (true) {
//    if (jQuery('div.form-item-citation-'+num).length == 0) {
//      break;
//    } else {
//      num++;
//    }
//  }
//  if (typeof i != 'undefined' && typeof name != 'undefined') {
//    if (i >= num) {
//      makeCitationCopy(num, name);
//    } else {
//      jQuery('div.form-item-citation-'+i+' input').val(name);
//    }
//  } else {
//    makeCitationCopy(num);
//  }
//
//  return false;
//}
//
//function makeCitationCopy(num, name) {
//  var copy = jQuery('div.form-item-citation-1').clone();
//  if (typeof name != 'undefined') {
//    copy.find('input').val(name);
//  } else {
//    copy.find('input').val('');
//  }
//  copy.removeClass('form-item-citation-1').addClass('form-item-citation-'+num);
//  copy.find('label').attr('for', 'edit-citation-'+num);
//  copy.find('#edit-citation-1').attr('id', 'edit-citation-'+num).attr('name', 'citation_'+num);
//
//  jQuery('#edit-citation-adder').before(copy);
//}
