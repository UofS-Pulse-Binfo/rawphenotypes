/**
 * @file
 * Manage behavior of project filter field.
 */
(function($) {
  Drupal.behaviors.adminProjectFilter = {
    attach: function (context, settings) {
      // Project sorted by types.    
      var typesProject = Drupal.settings.rawpheno.types_project;
      // Reference experiment filter field.
      var fldExperimentTypes = $('#admin-sel-types');
      // Reference project seleft field.
      var fldExperiment = $('#admin-sel-project');
      // Copy of all project when page first loaded.
      var allProject = Drupal.settings.rawpheno.all_project;
      var allProjectKeys = Object.keys(allProject);
      // Default type - Research Experiment.
      var defaultType = Drupal.settings.rawpheno.default_type;
      
      if (defaultType > 0) {
        $('#admin-sel-types option[value="'+ defaultType +'"]').attr('selected', true);
        populateSelect(defaultType);
      }

      fldExperimentTypes.change(function(e) {
        // Seleted type.
        var typeVal = parseInt(e.target.value);
        populateSelect(typeVal);
      });


      /**
       * Repopulate select field. 
       */
      function populateSelect(typeVal) {
        // RESET OPTIONS.
        $('#admin-sel-project option').remove();

        // CONSTRUCT NEW SET OF OPTIONS.
        // Filtered/all projects.
        var exp = (typeVal > 0) ? typesProject[ typeVal ].sort() : allProjectKeys;
                
        if (exp.length > 0) {
          // First option is a blank.
          fldExperiment.append($('<option>', {
            value: 0,
            text: '---',
            selected: true
          }));
          
          // Experiments options.
          exp.forEach(function(i) {
            i = parseInt(i);

            if (allProject[ i ]) {
              fldExperiment.append($('<option>', {
                value: i,
                text: allProject[ i ]
              }));
            }
          });
        }
      }
      // 
}};}(jQuery));
