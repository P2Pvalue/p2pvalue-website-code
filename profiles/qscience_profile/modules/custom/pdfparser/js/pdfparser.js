var authors_to_edit = new Array();
var citations_to_edit = new Array();
var field_num;
var i;

(function ($) {
  
  $(document).ready(function() {
    $('span.remove_button').live('click', function() {
      var val = $(this).parent().find('input[type="text"]').val();
      var type = $(this).hasClass('author') ? 'author' : 'citation';
      $(this).parent().remove();
      
      field_num = $('#'+type+'-reference-values tbody input.form-text').length;
      var added = false;
      for (i = 0; i < field_num; i++) {
        if ($('#'+ type +'-reference-values tbody input.form-text:eq('+ i +')').val().length == 0) {
          added = true;
          $('#'+ type +'-reference-values tbody input.form-text:eq('+ i +')').val(val);
//          console.log('setting value...' + val);
//          console.log('#'+ type +'-reference-values tbody #edit-'+ type +'-reference-und-'+ i +'-target-id');
          break;
        }
      }
      if (!added) {
//        console.log('not added');
        $('input[name="'+type+'_reference_add_more"]').trigger('mousedown');
        if(type == 'author') {
          authors_to_edit.push(val);
        } else if (type == 'citation') {
          citations_to_edit.push(val);
        }
      } else {
//        console.log('already added...somewhere....');
      }
    });
  });
  
  $(document).ajaxComplete(function(e, xhr, settings) {
    if (typeof settings.extraData != 'undefined') {
      console.log(settings);
      var i;
      if (settings.extraData._triggering_element_name == 'field_paper_upload_und_0_upload_button'
              || settings.extraData._triggering_element_name == 'field_paper_upload_und_0_transfer') {
        // Removing previous field datas.
        removeFieldValues();
        
        // Inserting new datas.
        if (Drupal.settings && Drupal.settings.pdfparser) {
          if (Drupal.settings.pdfparser.title) {
            $('#edit-title').val(Drupal.settings.pdfparser.title);
          }
          if (Drupal.settings.pdfparser.abstr) {
            $('#edit-abstract-und-0-value').val(Drupal.settings.pdfparser.abstr);
          }
          if (Drupal.settings.pdfparser.authors) {
            var authors = Drupal.settings.pdfparser.authors;
            var author_num = authors.length;
            if (author_num > 0) {
              for (i = 0; i < author_num; i++) {
                var author_row = $('fieldset.extracted_authors div.author_row.prototype').clone();
                author_row
                  .removeClass('prototype')
                  .find('input.author_name')
                    .val(authors[i])
                    .attr('name', 'author_reference[extracted]['+ i +'][name]');
                $('fieldset.extracted_authors div.fieldset-wrapper').append(author_row);
              }
            }
          }
          if (Drupal.settings.pdfparser.citations) {
            var citations = Drupal.settings.pdfparser.citations;
            var citation_num = citations.length;
            if (citation_num > 0) {
              for (i = 0; i < citation_num; i++) {
                var citation_row = $('fieldset.extracted_citations div.citation_row.prototype').clone();
                citation_row
                  .removeClass('prototype')
                  .find('input.citation')
                    .val(citations[i].title + getCitationAuthorString(citations[i].authors))
                    .attr('name', 'citation_reference[extracted]['+ i +'][name]');
                $('fieldset.extracted_citations div.fieldset-wrapper').append(citation_row);
              }
            }
          }
          delete Drupal.settings.pdfparser;
        }
      } else if (settings.extraData._triggering_element_name == 'field_paper_upload_und_0_remove_button') {
        removeFieldValues();
        $('#edit-field-paper-upload div.messages').slideUp(400);
      } else if (settings.extraData._triggering_element_name == 'author_reference_add_more') {
        if (authors_to_edit.length != 0) {
          field_num = $('#author-reference-values tbody input.form-text').length;
          $('#author-reference-values tbody input.form-text:eq('+ (field_num - 1) +')').val(authors_to_edit[0]);
          authors_to_edit.splice(0, 1);
          if (authors_to_edit.length != 0) {
            $('input[name="author_reference_add_more"]').trigger('mousedown');
          }
        }
      } else if (settings.extraData._triggering_element_name == 'citation_reference_add_more') {
        if (citations_to_edit.length != 0) {
          field_num = $('#citation-reference-values tbody input.form-text').length;
          $('#citation-reference-values tbody input.form-text:eq('+ (field_num - 1) +')').val(citations_to_edit[0]);
          citations_to_edit.splice(0, 1);
          if (citations_to_edit.length != 0) {
            $('input[name="citation_reference_add_more"]').trigger('mousedown');
          }
        }
      }
    }
  });
  
  function removeFieldValues() {
    $('#edit-title').val('');
    $('#edit-abstract-und-0-value').val('');
    $('fieldset.extracted_authors div.author_row').not('.prototype').remove();
    $('fieldset.extracted_citations div.citation_row').not('.prototype').remove();
    
    var types = ['author', 'citation'];
    
    for (var type in types) {
      var t = types[type];
      field_num = $('#'+ t +'-reference-values tbody input.form-text').length;
      for (i = 0; i < field_num; i++) {
        $('#'+ t +'-reference-values tbody input.form-text:eq('+ i +')').val('');
        if (i > 0) {
          $('#'+ t +'-reference-values div.form-type-checkbox input.form-checkbox:eq('+ i +')').trigger('change');
        }
      }
    }
  }
})(jQuery);



function getCitationAuthorString(authors) {
  var s = '';
  if (authors && authors.length > 0) {
    s = ' [Authors: '+ authors.join(', ') +']';
  }
  return s;
}
