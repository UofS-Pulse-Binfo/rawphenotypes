/**
 * @file
 * Create graphical representation of phenotypic data.
 */
(function($) {
  Drupal.behaviors.rawphenoRawDataCreateHeatMap = {
    attach: function (context, settings) {
      ////
        // Infobox container. Shows information about a particular rep
        // on mouseover.
        var infoBox = d3.select('body')
          .append('div')   
          .attr('class', 'tool-tip')               
          .style('opacity', 0);
            
        // Colour range is the associated colour for every trait count in the data.
        // initial colour is the initial colour of the chart before animation fills
        // each rectangle with the right colour.
        var startColour = 'green'; 
        var color = d3.scale.linear()
          .domain([0, 5, 10, 15, 20, 25, 30])
          .range(['#EAEAEA', '#E2EFDA', '#C6E0BA', '#A9D08E', '#70AD47', '#548235', '#375623']);
          
        // Canvas, data and count.
        var svg, data, numberOfLocations, numberOfReps, numberOfYears;
        
        // Dimension and margin.
        var height, width, margin = {};
        
        // x axis scale.
        var x0, xAxis;
        
        // Chart margins.
        margin.top    = 40;
        margin.left   = 85;
        margin.bottom = 80;
        margin.right  = 30;
        
        // Height of the chart defined in the css rule for #container-rawdata.
        height = parseInt(d3.select('#container-rawdata').style('height'), 10);
        
        // Add event listener when user resizes the window.
        d3.select(window).on('resize', render);
   
        // Initialize chart by adding all g, rect and text to the DOM.
        // The render function will position everything to the right
        // place once width is determined.
        var file = $('#rawdata-json').val();
        d3.json(file, function(error, data) {
          if (error) {
            // Error reading JSON
          }
          else if(data == 0) {
            // No data.
            d3.select('.data-chart')
              .append('g')
              .attr('width', 400)
              .append('text')
              .attr('y', 30)
              .attr('x', 120)
              .attr('font-size', '20px')
              .attr('fill', 'black')
              .text('No Data');
          }
          else {
            // Generate Chart.
            initializeChart(data)
          }
        });
        
        function initializeChart(data) {
          // Define the chart height.
          var chartHeight = height - margin.top - margin.bottom;
        
          // Text information.
          var chartTitle = 'Number of Trials per Location';
          var chartXTitle = 'Locations';
          var chartYTitle = 'Growing Season (year)';
          var chartLegendTitle = 'Number of traits';
          
          // Main svg canvas.
          svg = d3.select('.data-chart');
          
          //Title.
          // Add standard title, scales, text label and legend.
          var captions = svg.append('g').attr('id', 'g-captions');
          captions.append('text')
            .attr('id', 'standard-chart-title')
            .attr('class', 'chart-title')
            .text(chartTitle);
          
          // Growing season - y axis
          captions.append('text')
            .attr('class', 'chart-axes')
            .attr('transform', function() {
              return 'translate(35, '+ Math.round(height/2) +') rotate(-90)';
            })
            .text(chartYTitle);          
          
          // Location - x axis
          captions.append('text')
            .attr('id', 'location-title')
            .attr('class', 'chart-axes')
            .text(chartXTitle);

          // Scales.
          // X axis (locations)
          x0 = d3.scale.ordinal();
          xAxis = d3.svg.axis().orient('bottom');

          // Y axis (year)
          var y0 = d3.scale.ordinal()
            .rangeRoundBands([0, chartHeight]);
          var yAxis = d3.svg.axis()
            .scale(y0)
            .orient('left');
          
          // Legend.
          var legend = svg.append('g')
            .attr('id', 'g-legend')
            .attr('transform', 'translate(' + margin.left + ', '+ ((height - margin.top) + 20) +')');
    
          legend.append('text')
            .attr('class', 'legend-text')
            .text(chartLegendTitle)
            .attr('y', -5);        

          var j = 0;
          for(var m = 0; m <= 30; m = m+5) {
            legend.append('rect')
              .attr('transform', 'translate('+ (31 * j) +', 0)')
              .attr('width', 30)
              .attr('height', 7)
              .attr('fill', color(m));
          
            legend.append('text')
              .attr('class', 'legend-text')
              .attr('x', (30 * j) + 20)
              .attr('text-anchor', 'right')
              .attr('y', 17)
              .attr('width', 20)
              .text(m);  
            j++;  
          }

          // Group data with location as primary key.
          // year as secondary key and rep as tertiary.
          // data /location/year/rep - values.
          dataByLocation = d3.nest()
            .key(function(d) { return d.location; })
            .key(function(d) { return d.year; })
            .sortKeys(d3.descending)
            .key(function(d) { return d.rep; })
            .sortKeys(d3.sort)
            .rollup(function(v) { 
              return d3.sum(v, function(d) { return d.trait; });
            })
            .entries(data); 
	      
	        var dataByRep = d3.nest()
            .key(function(d) { return d.rep; })
            .entries(data);
        
          var dataByYear = d3.nest()
            .key(function(d) { return d.year })
            .sortKeys(d3.descending)
            .entries(data); 
	        
	        // Number of locations.
	        numberOfLocations = dataByLocation.length;
	        
	        // Number of reps per year.
	        numberOfReps = dataByRep.length;
	        
	        // Number of year in a location.
	        numberOfYears = dataByYear.length;
          
	        // Rect height of each rep.
          var barHeight = Math.round(chartHeight/numberOfYears);

          // Main chart wrapper.  
          var chartWrapper = svg.append('g')
            .attr('class', 'chart-wrapper')
	          .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
	          
          // Each location.  
          var locationWrapper = chartWrapper
            .selectAll('g')
            .data(dataByLocation)
            .enter()
              .append('g')
              .attr('class', 'g-each-location')
              .attr('id', function(d, i) {return 'location-' + i; });
	 
          // Each year.
          var yearWrapper = locationWrapper.selectAll('g')
            .data(function(d) { return d.values })
            .enter()
            .append('g')
              .attr('class', 'g-each-year')
              .attr('id', function(d) { return 'year-' + d.key; })
              .attr('transform', function(d, i) {
                return 'translate(0, '+ (i * barHeight) +')';
              });
     
          // Each rep in a year.
          yearWrapper.selectAll('rect')
            .data(function(d) { return d.values; })
            .enter()
            .append('g')
            
            .on('mousemove', function(d) {
              d3.select(this)
                .style('opacity', 0.5);
              infoBox
                .transition()
                .style('opacity', 1);
              infoBox
                .html('Rep ' + d.key + ': '+ d.values + ' Traits')  
                .style('left', (d3.event.pageX + 10) + 'px')     
                .style('top', (d3.event.pageY) + 'px');    
            })                  
            .on('mouseout', function(d) {       
              d3.select(this)
                .style('opacity', 1);  
              infoBox.transition()        
                .style('opacity', 0);   
            })
            
            .append('rect')
              .attr('class', 'rect-each-rep')
              .attr('stroke', '#FFFFFF')
              .attr('fill', startColour)
              .transition()
              .duration(function(d, i) { return 500 * i; })
              .ease('back')
              .attr('fill', function(d) { return color(parseInt(d.values)); })
              .attr('height', barHeight)
              .attr('y', 0);
	      
	        // Wrap each location.
	        locationWrapper.append('rect')
            .attr('class', 'rect-each-location')
            .attr('stroke', '#000000')
            .attr('stroke-width', 2)
            .attr('fill', 'none')
           .attr('height', barHeight * numberOfYears); 

          // Add Scales.
          // x axis (location).
          x0.domain(dataByLocation.map(function(d) { return d.key; }));
          chartWrapper.append('g')
            .attr('id', 'g-x-axis')
            .attr('class', 'axis')
            .attr('transform', 'translate(0,' + barHeight * numberOfYears +')');
            
	        // y axis (year).
          y0.domain(dataByYear.map(function(d) { return d.key; }));
          chartWrapper.append('g')
            .attr('class', 'axis')
            .attr('transform', 'translate(1,0)')
            .call(yAxis);

	        
	        // Update width, scales and dimensions.
	        render();
        }
        
        // With all elements in the DOM, position elements in the right place
        // relative to the width of the container div.
        function render() {
          // Get the width of the container div of the main svg canvas.
          updateWidth();
          
          // Define chart width based on the width generated.
          var chartWidth = width - margin.left - margin.right;
         
	        // Compute dimensions.
          var locationContainerWidth = Math.round(chartWidth/numberOfLocations);
          var barWidth = Math.round(locationContainerWidth/numberOfReps);
	       
	        // Render main canvas.
          svg
           .attr('width', width)
           .attr('height', height);
	        
          // Render each Location.
          d3.selectAll('.g-each-location')
            .attr('transform', function(d, i) {
              return 'translate('+ (i * locationContainerWidth) +', 0)';      
            }); 
	        
          // Render each rep. 
          var x = 0;
          d3.selectAll('.rect-each-rep')
            .attr('width', barWidth)
            .attr('x', function(d, i) { 
              if (i%numberOfReps == 0)
                x = 0;
              else 
                x++;
             
              return x * barWidth; 
            });  
          
          // Render wrapper for each location.
          d3.selectAll('.rect-each-location')
            .attr('width', locationContainerWidth)
           
          // Render standard title.
          d3.select('#standard-chart-title')
            .attr('transform', function() {
              return 'translate('+ Math.round(width/2) +', 15)';
            }); 
          
          // Render locations title.
          d3.select('#location-title')
            .attr('transform', function() {
              return 'translate('+ Math.round(width/2) + ', ' + (height - margin.top) +')';
            });
          
          // Render scales.
          x0.rangeRoundBands([0, chartWidth]);
          xAxis.scale(x0);
         
          d3.select('#g-x-axis').call(xAxis);
        }

        // Compute the width of the container div.
        function updateWidth() {
          // This is the width of the svg container.
          var containerWidth = d3.select('#container-rawdata').style('width');
          // Update the width to be used in redering the chart elements.
          width = parseInt(containerWidth, 10); 
        }
      ////
    }
  };
}(jQuery));