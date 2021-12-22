/**
 * Rawphenotypes in Germplasm Field.
 */
(function ($) {
  Drupal.behaviors.rawphenotypesGermplasm = {
    attach: function (context, settings) {
      // Raw phenotype definition.
      $('#rawphenotypes-germplasm-field-header div').eq(0).find('a').click(function(e) {
        e.preventDefault();
        $('#rawphenotypes-define-raw-container').slideDown(200);
      });

      // Okay - definition.
      if ($('#rawphenotypes-define-raw-container')) {
        $('#rawphenotypes-define-raw-container a').click(function(e) {
          e.preventDefault();
          $('#rawphenotypes-define-raw-container').slideUp(200);
        });
      }
      

      var imgOpacity = 0.5; 
      
      // Select box event - prepare link to download selection.
      var selects = $('#rawphenotypes-germplasm-field-table select');
      var downloadLink = '';

      selects.change(function(e) {                
        var selectValue = e.target.value;
        var selectId = e.target.id;
        downloadLink = '';

        // Reset other select to allow only one select field
        // at a time to export.
        selects.each(function() { 
          if ($(this).attr('id') != selectId) {
            $('#' + $(this).attr('id') + '-img').css('opacity', '0.5');
            this.selectedIndex = 0;
          }
        });

        if (selectValue == '0') {
          // None selected - default to select an option.
          imgOpacity = '0.5';
          // Detach event created.
        }
        else {
          // Selected, prepare query string for export.
          imgOpacity = '1';
          var params = selectValue.split('#');
          // Project id & location & trait.
          downloadLink = 'p=' + params[1] + '&l=' + params[2] + '&t=' + params[0];
        }

        $('#' + selectId + '-img').css('opacity', imgOpacity);
      });
       
      // Listen to images clicked to launch data download.
      $('#rawphenotypes-germplasm-field-table td:last-child img').click(function(e) {
        var imgId = e.target.id;
        var imgOpacity = $(this).css('opacity');

        if (imgOpacity == 1) {
          window.open(
            Drupal.settings.rawpheno.exportLink + '?code=' + btoa(downloadLink),
            '_blank'
          );
        }
      });
}};}(jQuery));