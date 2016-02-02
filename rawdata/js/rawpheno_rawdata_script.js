/**
 * @file
 * Create graphical representation of phenotypic data.
 */
(function($) {
  Drupal.behaviors.rawphenoRawData = {
    attach: function (context, settings) {
      $(document).ready(function() {
      /////
        //Default bg color of chart on load;
        var startColour = 'green';
        //Number of reps.  
        var rep = 3;
        
        //Colour map chart.
        var color = d3.scale.ordinal()
          .domain([0, 5, 10, 15, 20, 25, 30])
          .range(['#EAEAEA', '#E2EFDA', '#C6E0BA', '#A9D08E', '#70AD47', '#548235', '#375623']);
          
        //Bar/rect properties.
        var barBorder = 1;
        var barWidth = 30 - (barBorder * 2); //less 2px for borders
        var barHeight = 100 - (barBorder * 2); 
        
        //Margins and dimension
        var margin = {top: 30, right: 10, bottom: 20, left: 65};
        var height = 425 - margin.top - margin.bottom;
        var width  = 830 - margin.left - margin.right;
        
        //Tool tip container.
        var div = d3.select('body').append('div')   
          .attr('class', 'tool-tip')               
          .style('opacity', 0);
        
        //Main SVG canvas.
        var svg = d3.select('.data-chart')
        
        //Legend information.
        var legend = svg.append('g')
          .attr('transform', 'translate(' + margin.left + ', '+ (height + margin.top) +')')
          .attr('id', 'g-legend');
    
        legend.append('text')
          .attr('class', 'legend-text')
          .text('Number of traits')
          .attr('y', -5);        

        var j = 0;
        for(var m = 0; m <= 30; m = m+5) {
          legend.append('rect')
            .attr('transform', 'translate('+ (31 * j) +', 0)')
            .attr('width', 30)
            .attr('height', 8)
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
        //
        
        //Title and caption.
        var captions = svg.append('g').attr('id', 'g-captions');
        captions.append('text')
          .attr('class', 'chart-title')
          .attr('transform', function() {
            return 'translate('+ (center(width) + margin.left) +', 15)';
          })
          .text('Number of Trials per Location');
        
        //Growing season - y axis
        captions.append('text')
          .attr('class', 'chart-axes')
          .attr('transform', function() {
            return 'translate(10, '+ (center(height)) +') rotate(-90)';
          })
          .text('Growing Season (year)');
        
        //Location - x axis
        captions.append('text')
          .attr('class', 'chart-axes')
          .attr('transform', function() {
            //+20 px - past the x axis (locations)
            return 'translate('+ (center(width) + margin.left ) +', '+ (height - margin.top + 25) +')';
          })
          .text('Locations');
  
        //X axis (locations)
        var x0 = d3.scale.ordinal()
          .rangeRoundBands([0, width], .1);
        var xAxis = d3.svg.axis()
          .scale(x0)
          .orient('bottom');

        //Y axis (year)
        var y0 = d3.scale.ordinal()
          .rangeRoundBands([0, barHeight * rep], .1);
        var yAxis = d3.svg.axis()
          .scale(y0)
          .orient('left');

        //Create g on margin top and margin left position,
        //to contain chart
        var chart = svg
          .attr('height', height + margin.top + margin.bottom)
          .attr('width', width + margin.left + margin.right)
          .append('g')
            .attr('id', 'g-locations')
            .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
       
        //json file
        var file = $('#rawdata-json').val();
        //
        
        //Begin chart
        d3.json(file, function(error, data) {
          //Error reading file
          if (error) throw error;
          
          //Group data with location as primary key.
          //year as secondary key and rep as tertiary.
          //data /location/year/rep - values.
          var dataByLocation = d3.nest()
            .key(function(d) { return d.location; })
            .key(function(d) { return d.year; })
            .sortKeys(d3.descending)
            .key(function(d) { return d.rep; })
            .sortKeys(d3.ascending)
            .rollup(function(v) { 
              return d3.sum(v, function(d) { return d.trait; });
            })
            .entries(data);
          
          //Compute dimensions based on number of records to chart.
          var numberOfLocation = dataByLocation.length;
          var containerWidth = round(width/numberOfLocation);
          var barWidth = round(containerWidth/rep);
          
          //Container g for each location.
          // /location - read primary keys
          var g = chart.selectAll('g')
            .data(dataByLocation)
            .enter()
            .append('g')
              .attr('id', function(d, i) { return 'loc' + d.key; })
              .attr('transform', function(d, i) {
                 return 'translate('+ (i * containerWidth) +', 0)';      
              });

          //Container g for each year.
          // /location/year - read primary key then secondary key.
          var y = g.selectAll('g')
            .data(function(d) { return d.values })
            .enter()
            .append('g')
            .attr('id', function(d) { return 'year' + d.key; })
            .attr('transform', function(d, i) {
              return 'translate(0, '+ (i * barHeight) +')';
            });
          
          
          //Insert g and in it are 3 rect representing
          //3 reps done per year per location.
          // /location/year/rep - read primary key then secondary key 
          //followed by tertiary key.
          y.selectAll('rect')
            .data(function(d) { return d.values; })
            .enter()
            .append('g')
            .on('mouseover', function(d) {      
              div.transition()        
                .duration(200)      
                .style('opacity', 0.8);      
              div .html(d.values + ' Traits')  
                .style('left', (d3.event.pageX) + 'px')     
                .style('top', (d3.event.pageY) + 'px');    
            })                  
            .on('mouseout', function(d) {       
              div.transition()        
                .duration(500)      
                .style('opacity', 0);   
            })
            .append('rect')
            .attr('stroke', '#FFFFFF')
            .attr('stroke-width', barBorder)
            .attr('fill', startColour)
            .transition().duration(function(d, i) { return 500 * i; }).ease('back')
            .attr('fill', function(d) { return color(mapColour(type(d.values))); })
            .attr('width', barWidth)
            .attr('height', barHeight)
            .attr('x', function(d, i) { return i * barWidth; })
            .attr('y', 0)
            .attr('id', function(d) { return 't' + d.values; });
         
          //Wrap each location in rect with border.
          g.append('rect')
            .attr('stroke', '#000000')
            .attr('stroke-width', (barBorder + 1))
            .attr('fill', 'none')
            .attr('width', containerWidth)
            .attr('height', barHeight * 3);
         
          //Create data for y axis.
          var dataByYear = d3.nest()
            .key(function(d) { return d.year })
            .sortKeys(d3.descending)
            .entries(data);  
            
          //Add y axis (year).
          y0.domain(dataByYear.map(function(d) { return d.key; }));
          chart.append('g')
            .attr('class', 'axis')
            .attr('transform', 'translate(1,0)')
            .call(yAxis);
          
          //Add x axis (location).
          x0.domain(dataByLocation.map(function(d) { return d.key; }));
          chart.append('g')
            .attr('class', 'axis')
            .attr('transform', 'translate(0,' + (barHeight * 3) +')')
            .call(xAxis);
   
        });
         
        //List of functions required.
        
        //Function to convert to integer.
        function type(n) {
          return parseInt(n);
        }
       
        //Function to compute half/center position.
        function center(m) {
          return Math.round(m/2);
        }
        
        //Function to round of values.
        function round(n) {
          return Math.round(n);
        }
        
        //Map number of trait to colour code.
        function mapColour(c) {
          var ccode;
          
          if(c == 0) {
            ccode = 0
          }
          else if(c >= 1 && c <= 5) {
            ccode = 5;
          }
          else if(c >= 6 && c <= 10) {
            ccode = 10;
          }
          else if(c >= 11 && c <= 15) {
            ccode = 15;
          }
          else if(c >= 16 && c <= 20) {
            ccode = 20;
          }
          else if(c >= 21 && c <= 25) {
            ccode = 25
          }
          else if(c >= 26) {
            ccode = 30;
          }
          
          return ccode;
        }
      /////    
      });
    }
  };
}(jQuery));