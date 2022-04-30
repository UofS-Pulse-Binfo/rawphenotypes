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
    
      // Default select fields to Location + Experiment filter.
      selectOptions('le'); 
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
        selectReset(selectId);

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
        var imgOpacity = $(this).css('opacity');

        if (imgOpacity == 1) {
          window.open(
            Drupal.settings.rawpheno.exportLink + '?code=' + btoa(downloadLink),
            '_blank'
          );
        }
      });

      // Image to download ALL for a trait.
      $('#rawphenotypes-germplasm-field-table td:first-child img').click(function(e) {
        var imgId = e.target.id;
        var i = imgId.split('-');
        // rawphenotypes-germplasm-field-filterby-%s-img
        var downloadLink = 't=' + i[4] + '&p=All&l=All&g=' + Drupal.settings.rawpheno.germ;

        window.open(
          Drupal.settings.rawpheno.exportLink + '?code=' + btoa(downloadLink),
          '_blank'
        );
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
      $('#rawphenotypes-germplasm-controls-selectby a').click(function(e) { 
        e.preventDefault();  

        var selByStates = ['Experiment', 'Location + Experiment'];
        
        // If link is experiment - switch it to location + experiment
        // and vice versa.
        var selByText = ($(this).text() == 'Experiment') ? 1 : 0;
        var selByCur  = (selByText) ? 0 : 1; 
        
        $(this).text(selByStates[ selByText ]);
        $(this).parent().find('span').text(selByStates[ selByCur ]);
        
        // Match selet field.
        selectReset();
        var sel = (selByCur) ? 'le' : 'e';  
        selectOptions(sel);
      });

      /**
       * Reset select boxes and export link.
       * 
       * @param selectId 
       *   Trait id number used to reference a select field
       *   If this is specified, exclude this select from reset operation. 
       */
      function selectReset(selectId = -1) {
        var selects = $('#rawphenotypes-germplasm-field-table select');

        selects.each(function() { 
          if ($(this).attr('id') != selectId) {
            $('#' + $(this).attr('id') + '-img').css('opacity', '0.5');
            this.selectedIndex = 0;
          }
        });
      }
      
      /**
       * Remove option element from a select field.
       * @param select
       *   Object, reference to select element. 
       */
      function removeOptions(select) {
        select.find('option').each(function(i, v) {
          if (i > 0) $(this).remove();
        });
      }

      /**
       * Create select options
       * 
       * @param set
       *   String, indicate if options is for location+experiment or experiment.
       *   Default to le = location + experiment.
       */
      function selectOptions(set = 'le') {
        var dataset = Drupal.settings.rawpheno;
        var germplasm = dataset.germ;

        // Prepare dataset.
        if (set == 'le') {
          // LOCATION + EXPERIMENT:
          $.each(dataset.germRawdata, function(index, value){
            var trait = index.split('_');
            var element = $('#rawphenotypes-germplasm-field-filterby-' + trait[1]);
            removeOptions(element);
            $.each(value, function(i, v) {
              var disabled = v['phenotype_customfield_terms:user_experiment'] == 1 ? '' : 'disabled'; 
              element.append($('<option>', { 
                value: trait[1] + '#' + v['phenotype_customfield_terms:id'] + '#' + v['phenotype_customfield_terms:location'] + '#' + germplasm, 
                text : v['phenotype_customfield_terms:location'].toUpperCase() + '/' + v['phenotype_customfield_terms:name'],
                disabled: disabled
              }));
            });
          });
        }
        else if(set == 'e') {
          // EXPERIMENT:
          $.each(dataset.germRawdata, function(index, value){
            var expCache = new Array();
            var trait = index.split('_');
            var element = $('#rawphenotypes-germplasm-field-filterby-' + trait[1]);
            removeOptions(element);
            $.each(value, function(i, v) {
              var expId = v['phenotype_customfield_terms:id'];
              if (!expCache.includes(expId)) {
                var l = value.filter(function(v) { return v['phenotype_customfield_terms:id'] == expId });
                var ql = l.map(function(v) { return v['phenotype_customfield_terms:location']; });
                expCache.push(expId);
                
                var disabled = v['phenotype_customfield_terms:user_experiment'] == 1 ? '' : 'disabled';
                element.append($('<option>', { 
                  value: trait[1] + '#' + v['phenotype_customfield_terms:id'] + '#' + ql.join('+') + '#' + germplasm, 
                  text : v['phenotype_customfield_terms:name'] + ' (' + l.length + ' Locations)',
                  title: ql.join(' + '),
                  disabled: disabled
                }));
              } 
            });
          });
        }
      }

}};}(jQuery));