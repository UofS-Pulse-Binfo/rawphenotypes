/**
 * @file
 * Manage behaviors in instructions page.
 */
(function($) {
  Drupal.behaviors.rawphenoInstructionsTab = {
    attach: function (context, settings) {
      $(document).ready(function() {
      /////  
        //file
        var gallery = new Array(0, 1);
		    //caption
		    var caption = new Array(0, 1);
		    gallery[0] = ['01-tendrils-no-elongation', 
		                  '02-tendrils-no-elongation', 
		                  '01-tendrils-elongation'];
		    
		    caption[0] = ['No elongation', 
		                  'No elongation', 
		                  'Elongation'];
		    
		    //
		    gallery[1] = ['01-pods-emerged', 
		                  '02-pods-emerged', 
		                  '01-pods-variation', 
		                  '02-pods-variation']; 
		    
		    caption[1] = ['Freshly emerged pods', 
		                  'Freshly emerged pods', 
		                  'Sample of the pod variation', 
		                  'Sample of the pod variation'];
		
        var imgPath = $('#path').val();
		
		    $('#fragment-5 a').click(function(){
	        var topic = $('#fragment-5 select').val();
		      var curImg = $('#cur-img').val();
		      
		      curImg = parseInt(curImg);
          //prev and next image link
          var showImg = ($(this).text() == '<') ? curImg - 1 : curImg + 1;
		      
		      //load image and caption
		      if (gallery[topic][showImg]) {
			      $('#fragment-5 img').attr('src', imgPath+gallery[topic][showImg]+'.jpg'); 
			      $('#fragment-5 em').contents().replaceWith(caption[topic][showImg]); 
			      $('#cur-img').val(showImg);
		      }
		    });
		
		    $('#fragment-5 select').change(function(){ 
	        var topic = $(this).val();
		      $('#fragment-5 img').attr('src', imgPath+gallery[topic][0]+'.jpg');
		      $('#fragment-5 em').contents().replaceWith(caption[topic][0]); 
		      $('#cur-img').val(0);
		    });
		
		    //ref to txt field
        var txtField = $('#edit-search');
        
        //include field label as the default value of the field
        txtField.focus(function() {  
        if (this.value == 'Search Trait') { 
          this.value = ''; 
        }}).blur(function() {
          if (this.value == '') {
          this.value = 'Search Trait';
        }});

        //start tabs ui and enable tab view button
        $('#tabs').tabs({delay:0}); 
        
        //traits array
        var availableTrait = [
            //essentials 0 - 9
            'Planting Date (date)',
            'Days to Emergence (days)',
            '# Emerged Plants (count)',
            'Days Till 10% of Plants Have Elongated Tendril (days)',
            'Days Till 10% of Plants Have One Open Flower (R1; days)',
            'Days Till 10% of Plants Have Pods (R3; days)',
            'Days Till 10% of Plants Have Swollen Seeds in Pods (R5; days)',
            'Days Till 10% of Plants Have 1/2 Their Pods Mature (R7; days)',
            'Days Till Harvest (days)',
            'Diseases Present (y/n/?)',
            //optional 10 - 19
            'Number of Nodes on Primary Stem at R1 (count)',
            'R7 Trait: Lowest Pod Height (cm)',
            'R7 Traits: Canopy Height (cm)',
            'R7 Traits: Canopy Width (cm)',
            'R7 Traits: Plant Length (cm)',
            'Lodging (Scale: 1-5)',
            'Straw Biomass (g)',
            'Total Seed Mass (g)',
            'Total Number of Seeds (count)',
            '100 Seed Mass (g)',
            //subset traits 20 - 22
            'Subset Traits: # Peduncles (count)',
            'Subset Traits: # Pods (count)',
            'Subset Traits: # Seeds (count)'];
        
        //ref to search button
        var btnSearch = $("#btn_submit");
        btnSearch.click(function() { 
          if (txtField.val() == null || txtField.val().trim() == '') {
            //field is empty
            alert('Trait field is empty');
          }
          else {
            //searching trait keyword
            if ( btnSearch.val() == 'Search' ) {
              //get the index of selected trait
              var traitIndex = null;
              for( var i = 0; i < availableTrait.length; i++ ) {
                if (availableTrait[i].toLowerCase() == txtField.val().toLowerCase()) {
                  traitIndex = i;
                  break;
                }
              }
              //
              
              if( traitIndex != null ) {
                //go search
                btnSearch.val('Clear search');
              
                //figure the tab and trait category
                var traitCategory = '';
                if (traitIndex >= 0 && traitIndex <= 9) {
                  traitCategory = 'Essential Trait';
                } 
                else if (traitIndex >= 10 && traitIndex <= 19) {
                  traitCategory = 'Optional Trait';
                } 
                else {
                  traitCategory = 'Subset Trait';
                } 
                
                //find the table row with this trait in
                var traitType = '<p>* This trait is '+traitCategory+'</p>';
                var countLi = $('#tabs tr').size();
                for(var x = 0; x < countLi; x++) {
                  var m = $('#tabs tr').eq(x).find('div').text(); 
                  if (m == txtField.val()) {
                    var traitTrIndex = x;
                    break;
                  }
                } 
                
                //create the search result
                var traitRow = $('#tabs table').find('tr').eq(traitTrIndex).html();
                var tableHeader = $('table').eq(1).find('tr').eq(0).html();
                var newDiv = traitType+'<table><tr>'+tableHeader+'</tr><tr>'+traitRow+'</tr></table>';
                $('#container-option').append(newDiv);
              }
              else {
                //not found
                alert('Trait '+txtField.val()+' not found');
              }            
            }
            else {
              //reset search
              btnSearch.val('Search');
              txtField.val('Search Trait');
              //remove the search result
              $('#container-option table, #container-option p').hide('slow').remove();
            }  
          }  
        });
      /////    
      });
    }
  };
}(jQuery));