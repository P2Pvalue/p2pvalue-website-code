(function ($) {
  $(document).ajaxComplete(function(e, xhr, settings) {
    if (typeof settings.extraData != 'undefined') {
      var field_num;
      var i, j;
      if (settings.extraData._triggering_element_name == 'field_paper_upload_und_0_upload_button') {
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
            field_num = $('#author-reference-values tbody input.form-text').length;
            var authors = Drupal.settings.pdfparser.authors;
            var author_num = authors.length;
            for (i = 0, j = 0; i < author_num; j++) {
              if (j >= field_num) {
                break;
              } else {
                $('#author-reference-values tbody #edit-author-reference-und-'+ j +'-target-id').val(authors[i]);
                Drupal.settings.pdfparser.authors.splice(i, 1);
              }
            }
            if (Drupal.settings.pdfparser.authors.length != 0) {
              $('#edit-author-reference-und-add-more').trigger('mousedown');
            }
          }
          if (Drupal.settings.pdfparser.citations) {
            field_num = $('#citation-reference-values tbody input.form-text').length;
            var citations = Drupal.settings.pdfparser.citations;
            var citation_num = citations.length;
            for (i = 0, j = 0; i < citation_num; j++) {
              if (j >= field_num) {
                break;
              } else {
                $('#citation-reference-values tbody #edit-citation-reference-und-'+ j +'-target-id').val(citations[i].title + getCitationAuthorString(citations[i].authors));
                Drupal.settings.pdfparser.citations.splice(i, 1);
              }
            }
            if (Drupal.settings.pdfparser.citations.length != 0) {
              $('#edit-citation-reference-und-add-more').trigger('mousedown');
            }
          }
        }
      } else if (settings.extraData._triggering_element_name == 'field_paper_upload_und_0_remove_button') {
        removeFieldValues();
        $('#edit-field-paper-upload div.messages').slideUp(400);
      } else if (settings.extraData._triggering_element_name == 'author_reference_add_more') {
        if (Drupal.settings && Drupal.settings.pdfparser) {
          if (Drupal.settings.pdfparser.authors && Drupal.settings.pdfparser.authors.length > 0) {
            field_num = $('#author-reference-values tbody input.form-text').length;
            $('#author-reference-values tbody #edit-author-reference-und-'+ (field_num - 1) +'-target-id').val(Drupal.settings.pdfparser.authors[0]);
            Drupal.settings.pdfparser.authors.splice(0, 1);
            if (Drupal.settings.pdfparser.authors.length != 0) {
              $('#edit-author-reference-und-add-more').trigger('mousedown');
            }
          }
        }
      } else if (settings.extraData._triggering_element_name == 'citation_reference_add_more') {
        if (Drupal.settings && Drupal.settings.pdfparser) {
          if (Drupal.settings.pdfparser.citations && Drupal.settings.pdfparser.citations.length > 0) {
            field_num = $('#citation-reference-values tbody input.form-text').length;
            $('#citation-reference-values tbody #edit-citation-reference-und-'+ (field_num - 1) +'-target-id').val(
                    Drupal.settings.pdfparser.citations[0].title + getCitationAuthorString(Drupal.settings.pdfparser.citations[0].authors));
            Drupal.settings.pdfparser.citations.splice(0, 1);
            if (Drupal.settings.pdfparser.citations.length != 0) {
              $('#edit-citation-reference-und-add-more').trigger('mousedown');
            }
          }
        }
      }
    }
  });
  
  function removeFieldValues() {
    var field_num;
    $('#edit-title').val('');
    $('#edit-abstract-und-0-value').val('');
    field_num = $('#author-reference-values tbody input.form-text').length;
    for (i = 0; i < field_num; i++) {
      $('#author-reference-values tbody #edit-author-reference-und-'+ i +'-target-id').val('');
    }
    field_num = $('#citation-reference-values tbody input.form-text').length;
    for (i = 0; i < field_num; i++) {
      $('#citation-reference-values tbody #edit-citation-reference-und-'+ i +'-target-id').val('');
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
