import React from "react";
import { Bar } from "react-chartjs-2";
import { useEffect, useState } from "react";
import Chart from "chart.js/auto";

const Use_mem_top_procs = ({ip,port}) => {

    const [data, setData] = useState({
        labels: [], // Name and PID of the top processes
        datasets: [{
          label: '% Process Memory Usage',
          data: [], // Porcentage of memory usage of the top processes
          backgroundColor: [
            'rgba(255, 99, 132, 0.2)',
            'rgba(255, 159, 64, 0.2)',
            'rgba(255, 205, 86, 0.2)',
            'rgba(75, 192, 192, 0.2)',
            'rgba(54, 162, 235, 0.2)'
          ],
          borderColor: [
            'rgb(255, 99, 132)',
            'rgb(255, 159, 64)',
            'rgb(255, 205, 86)',
            'rgb(75, 192, 192)',
            'rgb(54, 162, 235)'
          ],
          borderWidth: 1
        }]
      });

      function update_pages() {
        const url = `http://${ip}:${port}/use_mem_top_procs`;
        fetch(url)
          .then((res) => {
            return res.json();
          })
          .then((use_mem_top_procs) => {
            let labels = [];
            let data = [];
            use_mem_top_procs.forEach((proc) => {
                labels.push(proc.name + " (" + proc.pid + ")");
                data.push(proc.pct_usage);
            });
            setData({
                labels: labels,
                datasets: [{
                  label: '% Process Memory Usage',
                  data: data,
                  backgroundColor: [
                    'rgba(255, 99, 132, 0.2)',
                    'rgba(255, 159, 64, 0.2)',
                    'rgba(255, 205, 86, 0.2)',
                    'rgba(75, 192, 192, 0.2)',
                    'rgba(54, 162, 235, 0.2)'
                  ],
                  borderColor: [
                    'rgb(255, 99, 132)',
                    'rgb(255, 159, 64)',
                    'rgb(255, 205, 86)',
                    'rgb(75, 192, 192)',
                    'rgb(54, 162, 235)'
                  ],
                  borderWidth: 1
                }]
            });
          })
          .catch((err) => console.log(err));
    }

    useEffect(() => {
        const interval = setInterval(() => {
            update_pages();
        }, 1000);
        return () => clearInterval(interval);
    }, []);

    return (
        <>
            <Bar data={data} />
        </>
    );
}

export default Use_mem_top_procs;