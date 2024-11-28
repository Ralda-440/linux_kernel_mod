import React from "react";
import { Pie } from "react-chartjs-2";
import { useEffect, useState } from "react";
import Chart from "chart.js/auto";

const Use_pages = ({ip,port}) => {

    const [data, setData] = useState({
        labels: ["Pages Actives", "Pages Inactives"],
        datasets: [
        {
        label: "Pages",
        backgroundColor: [
            'rgb(255, 99, 132)',
            'rgb(54, 162, 235)'
        ],
        data: [0, 0],
        },
        ],
    });

    function update_pages() {
        const url = `http://${ip}:${port}/use_pages`;
        fetch(url)
          .then((res) => {
            return res.json();
          })
          .then((use_pages) => {
            setData({
                labels: ["Pages Actives", "Pages Inactives"],
                datasets: [
                {
                label: "Pages",
                backgroundColor: [
                    'rgb(255, 99, 132)',
                    'rgb(54, 162, 235)'
                ],
                data: [use_pages.active, use_pages.inactive],
                },
                ],
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
            <Pie data={data} />
        </>
    );
}

export default Use_pages;