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
          downloadLink = 't=' + params[0] + '&p=' + params[1] + '&l=' + params[2] + '&g=' + params[3];
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

      // Listen to controls search, expand and select by.
      var tableWindow = $('#rawphenotypes-germplasm-export-table');
      // Create border when scrolling.
      tableWindow.scroll(function() {
        var c = ($(this).scrollTop() <= 0) ? '#FFFFFF' : '#314355';
        $('#rawphenotypes-germplasm-controls').css('border-bottom-color', c);
      });

      // Expand.
      $('#rawphenotypes-germplasm-controls-expand').click(function(){
        var h = ($(this).is(':checked')) ? 460 : 230;
        tableWindow.css('height', h);
      });

      // Search.
      $('#rawphenotypes-germplasm-controls-search').click(function(e) {
        e.preventDefault();

        $('#rawphenotypes-germplasm-controls-search-window')
          .fadeIn('fast')
          .find('input').val('').focus();

        tableWindow.scrollTop(-1);
      }); 
        
        // Close search window.
        $('#rawphenotypes-germplasm-controls-search-window a').click(function(e) {
          e.preventDefault();

          $(this)
            .parent().fadeOut('fast')
            .find('input').val('');
        });

        // Search field when selected/on focus.
        $('#rawphenotypes-germplasm-controls-search-window input')
        .click(function() {
          if ($(this).val()) {
            $(this).select();
          }
        });

        // Prepare search items (all traits currently displayed in the table).
        var searchTerms = new Array();
        $('#rawphenotypes-germplasm-export-table table td:nth-child(2)').each(function() {
          searchTerms.push($(this).text());
        });
        
        $('#rawphenotypes-germplasm-controls-search-window input')
        .autocomplete({
          select: function(event, ui) {
            var foundIndex = searchTerms.indexOf(ui.item.value.trim());
            var foundRow = $('#rawphenotypes-germplasm-export-table table tbody tr').eq(foundIndex);
            tableWindow.scrollTop(foundRow.position().top);

            var t = 0;
            var timer = setInterval(function() {
              if (t < 5) {
                var o = (t%2 == 0) ? 0 : 1;
                foundRow.css('opacity', o);
              } 
              else {
                foundRow.css('opacity', 1);
                clearInterval(timer);
              }

              t++;
            }, 250);

            $(this).parent().fadeOut();            
          },
          source: function(request, response) {
            var results = $.ui.autocomplete.filter(searchTerms, request.term);
            // Fist 5 results.
            response(results.slice(0, 5));
          }
        }); 
      
      // Select by type.



}};}(jQuery));