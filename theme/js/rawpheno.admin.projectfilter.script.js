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
      else {
        populateSelect(0);
      }

      fldExperimentTypes.change(function(e) {
        // Seleted type.
        var typeVal = e.target.value;
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
        var exp = ''; 
        exp = (typeVal != 0) ? typesProject[ typeVal ] : allProjectKeys;

        if (exp.length > 0) {
          // First option is a blank.
          fldExperiment.append($('<option>', {
            value: 0,
            text: '---',
            selected: true
          }));
          
          // Create an object of experiments.
          var expObj = [];
          exp.forEach(function(i) { 
            i = parseInt(i);

            if (allProject[ i ]) {
              expObj.push({'id': i, 'name': allProject[ i ]});  
            }
          });

          // Should name 1 be placed in name 2. 
          // name 1 vs name 2 - sort 2 objects at a time and put name 1 in name 2 when 1.
          sortName = expObj.sort((n1, n2) => (n1.name > n2.name) ? 1 : (n1.name < n2.name) ? -1 : 0);
          
          // Create options.
          sortName.forEach(e => {
            fldExperiment.append($('<option>', {
              value: e['id'],
              text: e['name']
            }));
          });
        }
      }
      // 
}};}(jQuery));