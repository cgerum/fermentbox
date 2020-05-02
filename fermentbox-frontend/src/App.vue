<template>
  <v-app>
    <v-app-bar app color="primary" dark>
      <v-btn icon @click="drawer = !drawer">
        <v-app-bar-nav-icon></v-app-bar-nav-icon>
      </v-btn>
      <div class="d-flex align-center">
        <v-icon large>mdi-bacteria</v-icon>
        <span style="font-size: 2em; margin-left: 5px">Ferment Box</span>
      </div>

      <v-spacer></v-spacer>

      <v-icon v-if="connected">mdi-wifi</v-icon>
      <v-icon v-else>mdi-wifi-off</v-icon>
    </v-app-bar>

    <v-content>
      <v-navigation-drawer v-model="drawer" absolute temporary>
        <v-list>
          <v-list-item link @click="setPage('controls')">
            <v-list-item-icon>
              <v-icon>mdi-file-table-box-outline</v-icon>
            </v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Controls</v-list-item-title>
            </v-list-item-content>
          </v-list-item>
          <v-list-item link @click="setPage('schedule')">
            <v-list-item-icon>
              <v-icon>mdi-timer-outline</v-icon>
            </v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Schedule</v-list-item-title>
            </v-list-item-content>
          </v-list-item>
          <v-list-item link @click="setPage('settings')">
            <v-list-item-icon>
              <v-icon>mdi-cogs</v-icon>
            </v-list-item-icon>
            <v-list-item-content>
              <v-list-item-title>Settings</v-list-item-title>
            </v-list-item-content>
          </v-list-item>
        </v-list>
      </v-navigation-drawer>

      <div v-show="page === 'controls'">
        <div class="page-title">
          <v-icon large color="accent" class="page-title-icon">mdi-file-table-box-outline</v-icon>
          <span>Controls</span>
        </div>
        <v-list>
          <v-list-item>
            <v-list-item-content>
              <temperature-control></temperature-control>
              <humidity-control></humidity-control>
            </v-list-item-content>
          </v-list-item>
        </v-list>
      </div>

      <div v-show="page === 'schedule'">
        <div class="page-title">
          <v-icon large color="accent" class="page-title-icon">mdi-timer-outline</v-icon>
          <span>Schedule</span>
        </div>
        <schedule></schedule>
      </div>

      <div v-show="page === 'settings'">
        <div class="page-title">
          <v-icon large color="accent" class="page-title-icon">mdi-cogs</v-icon>
          <span>Settings</span>
        </div>
        <settings></settings>
      </div>
    </v-content>
  </v-app>
</template>

<style lang="scss" scoped>
@import "./style/styles.scss";
.page-title {
  color: $color-accent;
  font-size: 1.6em;
  font-weight: bold;
  margin-left: 15px;
  margin-top: 15px;
}
.page-title-icon {
  margin-right: 5px;
}
</style>


<script>
import TemperatureControl from "./components/TemperatureControl";
import HumidityControl from "./components/HumidityControl";
import Schedule from "./components/Schedule";
import Settings from "./components/Settings";
import EventBus from "./event-bus.js"

export default {
  name: "App",

  components: {
    TemperatureControl,
    HumidityControl,
    Schedule,
    Settings
  },

  data: () => ({
    connected: true, // TODO check for connection
    drawer: false,
    page: "controls"
  }),

  created: function() {
    this.startMeasurement()
  },

  methods: {
    setPage(name) {
      this.page = name;
      this.drawer = false;
    },

    startMeasurement() {
      this.measurements = []
      setInterval(() => {
        sendRequest('measurement').then((data) => {
          this.measurements.push(data)
          EventBus.$emit('new-measurement', data)
        })
      }, 5000)
    }

  }
};

function sendRequest(url) {
    return new Promise((resolve, reject) => {
        const xhr = new XMLHttpRequest()
        xhr.onreadystatechange = () => {
            if (xhr.readyState === 4) {
                if( xhr.status === 200) {
                    resolve(JSON.parse(xhr.responseText))
                } else {
                    reject(xhr.statusText)
                }
            } 
        }
        xhr.open('GET', url, true)
        xhr.send()
    })        
}

</script>
