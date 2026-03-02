from arduino.app_bricks.video_imageclassification import VideoImageClassification
from arduino.app_utils import *
from arduino.app_bricks.web_ui import WebUI
from datetime import datetime, UTC
import json
import queue
import time

is_running = False
person_present = False
has_greeted = False

bridge_queue = queue.Queue()
bridge_queue.put("reset_state")

ui = WebUI()
classification_stream = VideoImageClassification(confidence=0.7, debounce_sec=5.0)
ui.on_message("override_th", lambda sid, threshold: classification_stream.override_threshold(threshold))

def person_detected():
    global is_running, person_present, has_greeted
    person_present = True
    if not is_running and not has_greeted:
        is_running = True
        has_greeted = True
        print("Person detected")
        bridge_queue.put("say_hello")

def no_person():
    global person_present, has_greeted
    if person_present:  # only act if person WAS present, not on every fire
        person_present = False
        if not is_running:
          has_greeted = False
          bridge_queue.put("show_neutral")
          
classification_stream.on_detect("person", person_detected)
classification_stream.on_detect("non person", no_person)

def send_classifications_to_ui(classifications: dict):
    if len(classifications) == 0:
        return
    entries = []
    for key, value in classifications.items():
        entry = {
            "content": key,
            "confidence": value,
            "timestamp": datetime.now(UTC).isoformat()
        }
        entries.append(entry)
    if len(entries) > 0:
        msg = json.dumps(entries)
        ui.send_message("classifications", message=msg)

classification_stream.on_detect_all(send_classifications_to_ui)

def loop():
    global is_running
    try:
        if not bridge_queue.empty():
            command = bridge_queue.get()
            print(f"Executing: {command}")
            Bridge.call(command)
            print(f"Finished: {command} | person_present: {person_present} | is_running: {is_running}")
            if command == "say_hello":
                is_running = False
                while not bridge_queue.empty():
                    bridge_queue.get()
                if not person_present:
                    print("Person not present after servo - showing neutral")
                    bridge_queue.put("show_neutral")
                  
    except Exception as e:
      print(f"ERROR: {e}")
      is_running = False
      while not bridge_queue.empty():
          bridge_queue.get()
              
App.run(user_loop=loop)