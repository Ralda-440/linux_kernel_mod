import React from "react";
import { Bar } from "react-chartjs-2";
import { useEffect, useState } from "react";
import Chart from "chart.js/auto";

const Fault_pages = ({ip,port}) => {

    const [data, setData] = useState({
        labels: [
          'Fault Minor',
          'Fault Major'
        ],
        datasets: [{
          type: 'bar',
          label: 'Fault Minor',
          data: [5,0],
          borderColor: 'rgb(255, 99, 132)',
          backgroundColor: 'rgb(255, 99, 132)'
        }, {
          type: 'bar',
          label: 'Fault Major',
          data: [0,5],
          fill: false,
          borderColor: 'rgb(54, 162, 235)',
          backgroundColor: 'rgb(54, 162, 235)'
        }]
      });

    function update_fault_pages() {
        const url = `http://${ip}:${port}/fault_pages`;
        fetch(url)
          .then((res) => {
            return res.json();
          })
          .then((fault_pages) => {
            setData({
                labels: [
                    'Fault Minor',
                    'Fault Major'
                  ],
                  datasets: [{
                    type: 'bar',
                    label: 'Fault Minor',
                    data: [fault_pages.minor,0],
                    borderColor: 'rgba(255, 99, 132, 0.2)',
                    backgroundColor: 'rgba(255, 99, 132, 0.2)'
                  }, {
                    type: 'bar',
                    label: 'Fault Major',
                    data: [0,fault_pages.major],
                    fill: false,
                    borderColor: 'rgba(255, 159, 64, 0.2)',
                    backgroundColor: 'rgba(255, 159, 64, 0.2)'
                    }]
            });
          })
          .catch((err) => console.log(err));
    }

    useEffect(() => {
        const interval = setInterval(() => {
            update_fault_pages();
        }, 1000);
        return () => clearInterval(interval);
    }, []);

    return (
        <>
            <Bar data={data} />
        </>
    );
}

export default Fault_pages;