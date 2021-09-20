import base64
import os
import sys
_dir = os.path.dirname(__file__)
sys.path.append(_dir)

import pandas as pd
import json
import re
import time
import uuid
from io import BytesIO
from pathlib import Path

import numpy as np
import streamlit as st
from PIL import Image
from streamlit_drawable_canvas import st_canvas
from svgpathtools import parse_path

import SessionState
import connector as cn



def main():
    handle = cn.Handler.getInstance()
    playground = st.sidebar.selectbox("Type", options=list(handle.bd.getGroundType().keys()))
    
    print(playground)
    handle.setGround(playground)

    if 'button_id' not in st.session_state:
        st.session_state['button_id'] = ''
    if 'color_to_label' not in st.session_state:
        st.session_state['color_to_label'] = {}
    """ 
    PAGES = {
        "Calibration": color_annotation_app,
        "Basic example": full_app,
    }
    page = st.sidebar.selectbox("Page:", options=list(PAGES.keys())) """
    #PAGES[page]()
    color_annotation_app()

    with st.sidebar:
        st.markdown("---")
        st.markdown(
            '<h6>Made by Kelly @4D </h6>',
            unsafe_allow_html=True,
        )


def color_annotation_app():
    st.markdown(
        """
        You can see one image.

        It is first Camera among camera group.
        
        Please Select ROI, 4 Region.
    """
    )
    groundtype = None
    region = []
    handle = cn.Handler.getInstance()
    img_list = handle.bd.getImageList()
    bg_image = Image.open(img_list[0])
    w, h = bg_image.size[:2]
    print("image size : ", h , w)

    label_color = (
        st.sidebar.color_picker("Annotation color: ", "#EA1010") + "77"
    )  # for alpha from 00 to FF
    label = st.sidebar.text_input("Label", "Default")
    mode = "transform" if st.sidebar.checkbox("Move ROIs", False) else "rect"

    canvas_result = st_canvas(
        fill_color=label_color,
        stroke_width=1,
        background_image=bg_image,
        height=h/2,
        width=w/2,
        drawing_mode=mode,
        key="color_annotation_app",
    )
    if st.button('Select Complete. GO!'):
        st.write('')
        handle.ExecuteExtract()        
    else:
        st.write('Will calculate images')

    if canvas_result.json_data is not None:
        df = pd.json_normalize(canvas_result.json_data["objects"])
        if len(df) == 0:
            return
        st.session_state["color_to_label"][label_color] = label
        df["label"] = df["fill"].map(st.session_state["color_to_label"])
        st.dataframe(df[["top", "left", "width", "height", "fill", "label"]])
        
        region.append([canvas_result.json_data["objects"][len(df)-1]["left"],
                        canvas_result.json_data["objects"][len(df)-1]["top"],        
                        canvas_result.json_data["objects"][len(df)-1]["width"],
                        canvas_result.json_data["objects"][len(df)-1]["height"]])


        handle.setRegion(region)

    with st.expander("Color to label mapping"):
        st.json(st.session_state["color_to_label"])
    

def png_export():
    st.markdown(
        """
    Realtime update is disabled for this demo. 
    Press the 'Download' button at the bottom of canvas to update exported image.
    """
    )
    try:
        Path("tmp/").mkdir()
    except FileExistsError:
        pass

    # Regular deletion of tmp files
    # Hopefully callback makes this better
    now = time.time()
    N_HOURS_BEFORE_DELETION = 1
    for f in Path("tmp/").glob("*.png"):
        st.write(f, os.stat(f).st_mtime, now)
        if os.stat(f).st_mtime < now - N_HOURS_BEFORE_DELETION * 3600:
            Path.unlink(f)

    if st.session_state["button_id"] == "":
        st.session_state["button_id"] = re.sub("\d+", "", str(uuid.uuid4()).replace("-", ""))

    button_id = st.session_state["button_id"]
    file_path = f"tmp/{button_id}.png"

    custom_css = f""" 
        <style>
            #{button_id} {{
                display: inline-flex;
                align-items: center;
                justify-content: center;
                background-color: rgb(255, 255, 255);
                color: rgb(38, 39, 48);
                padding: .25rem .75rem;
                position: relative;
                text-decoration: none;
                border-radius: 4px;
                border-width: 1px;
                border-style: solid;
                border-color: rgb(230, 234, 241);
                border-image: initial;
            }} 
            #{button_id}:hover {{
                border-color: rgb(246, 51, 102);
                color: rgb(246, 51, 102);
            }}
            #{button_id}:active {{
                box-shadow: none;
                background-color: rgb(246, 51, 102);
                color: white;
                }}
        </style> """

    data = st_canvas(update_streamlit=False, key="png_export")
    if data is not None and data.image_data is not None:
        img_data = data.image_data
        im = Image.fromarray(img_data.astype("uint8"), mode="RGBA")
        im.save(file_path, "PNG")

        buffered = BytesIO()
        im.save(buffered, format="PNG")
        img_data = buffered.getvalue()
        try:
            # some strings <-> bytes conversions necessary here
            b64 = base64.b64encode(img_data.encode()).decode()
        except AttributeError:
            b64 = base64.b64encode(img_data).decode()

        dl_link = (
            custom_css
            + f'<a download="{file_path}" id="{button_id}" href="data:file/txt;base64,{b64}">Export PNG</a><br></br>'
        )
        st.markdown(dl_link, unsafe_allow_html=True)



if __name__ == "__main__":
    st.set_page_config(
        page_title="GENESIS", page_icon=":earthl2:"
    )
    st.title("CALIBRATION SIMULATION")
    st.sidebar.subheader("Config")
    main()
