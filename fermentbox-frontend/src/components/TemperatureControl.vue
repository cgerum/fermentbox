<template>
  <v-card outlined>
    <v-card-title>
      <v-icon x-large color="secondary">mdi-thermometer</v-icon>
      <span class="card-title">Temperature <span>{{currentValue}}</span> °C</span>
    </v-card-title>
    <v-card-text>
    <v-sparkline
      :gradient="['#f72047', '#ffd200', '#1c449a']"
      :line-width="3"
      :smooth="16"
      :value="log"
      :labels="labels"
      auto-draw
      stroke-linecap="round"
    ></v-sparkline>
    </v-card-text>
  </v-card>
</template>

<style lang="scss" scoped>
@import "../style/styles.scss";
.card-title {
  color: $color-secondary;
}
</style>

<script>
import EventBus from '../event-bus.js'

export default {
  name: "TemperatureControl",

  data: () => ({
    temperature: 20,
    log: []
  }),
  computed: {
    labels: function() {
      return this.log.map(data => data.label);
    },
    currentValue: function() {
      return this.log.length > 0 ? this.log[this.log.length -1].value : "--"
    }
  },
  
  created: function() {
    EventBus.$on('new-measurement', data => {
      this.updateLog(data)
    })
  },

  methods: {
    updateLog: function(data) {
      if (this.log.length === 20) {
        this.log.shift()
      }
      this.log.push({label: data.date, value: data.temperature})
    }
  }
};
</script>
