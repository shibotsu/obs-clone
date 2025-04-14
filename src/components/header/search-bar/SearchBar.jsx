import { useState, useCallback } from "react";
import { useQuery } from "@tanstack/react-query";
import { SearchBox, Spinner, makeStyles } from "@fluentui/react-components";
import { useNavigate } from "react-router-dom";
import debounce from "lodash.debounce";
import FollowingSidebarItem from "../../sidebar/FollowingSidebarItem";

const useStyles = makeStyles({
  container: {
    position: "relative", // This makes absolutely positioned children relative to this container.
    width: "100%",
    maxWidth: "350px",
    marginTop: "0.4rem",
  },
  suggestionsList: {
    position: "absolute",
    top: "40px", // Slightly below the search box height
    left: "50%",
    transform: "translateX(-45%)",
    width: "98%", // match container width
    backgroundColor: "white",
    borderTop: "none",
    zIndex: 10,
    maxHeight: "200px",
    overflowY: "auto",
    boxShadow: "0 4px 8px rgba(0, 0, 0, 0.1)", // Optional for overlay feel
    borderRadius: "8px",
  },
  suggestionItem: {
    position: "relative",
    left: 0,
  },
  searchBar: {
    width: "110%",
  },
});

export const SearchBar = () => {
  const [query, setQuery] = useState("");
  const [debouncedQuery, setDebouncedQuery] = useState("");
  const classes = useStyles();
  const navigate = useNavigate();

  const debounceQuery = useCallback(
    debounce((value) => {
      setDebouncedQuery(value);
    }, 300),
    []
  );

  const handleSearchChange = (event, newValue) => {
    setQuery(newValue.value);
    debounceQuery(newValue.value);
  };

  const { data, isLoading } = useQuery({
    queryKey: ["searchResults", debouncedQuery],
    queryFn: () =>
      fetch("http://127.0.0.1:8000/api/search", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ query: debouncedQuery }),
      }).then((res) => res.json()),
    enabled: debouncedQuery.trim().length > 1, // only trigger call if query has more than 1 character
  });

  const suggestions = data === undefined || data === null ? [] : data.users;
  const topSuggestions = suggestions.slice(0, 5);

  const handleSuggestionClick = () => {
    setQuery("");
    setDebouncedQuery("");
  };

  const handleFullSearch = () => {
    navigate("/search-results", {
      state: { results: suggestions, query: query },
    });
    setQuery("");
    setDebouncedQuery("");
  };

  return (
    <div className={classes.container}>
      <SearchBox
        placeholder="Search channels..."
        value={query}
        onChange={handleSearchChange}
        onKeyDown={(event) => {
          if (event.key === "Enter") {
            handleFullSearch();
          }
        }}
        className={classes.searchBar}
      />

      <div className={classes.suggestionsList}>
        {isLoading && <Spinner size="tiny" />}
      </div>

      {topSuggestions.length > 0 && (
        <div className={classes.suggestionsList}>
          {topSuggestions.map((channel) => (
            <FollowingSidebarItem
              id={channel.id}
              avatar={channel.profile_picture}
              name={channel.username}
              onClick={handleSuggestionClick}
            />
          ))}
        </div>
      )}
    </div>
  );
};
